#include "Game.h"

#include "BertieBotts.h"
#include "Bot.h"
#include "Debug.h"
#include "Draw.h"
#include "Gingerbread.h"
#include "Item.h"
#include "Input.h"
#include "Inventory.h"
#include "Line.h"
#include "Map.h"
#include "MapGenerator.h"
#include "Menu.h"
#include "PerfTimer.h"
#include "Player.h"
#include "Potion.h"
#include "Random.h"
#include "Spawn.h"
#include "Spell.h"
#include "Stairs.h"
#include "Status.h"
#include "Target.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <filesystem>

namespace Game
{

int constexpr c_VersionNumber = 0;

int s_turn_number;
GameMode s_game_mode;
std::string s_filename;

//------------------------------------------------------------------------------
// Helper function declarations.
void end_turn();
void game_over();
void create_savefile();
void delete_savefile();

//------------------------------------------------------------------------------
// Interface function implementations

// Initialization is in several layers (init, clear, setup).
// Alphabetize the init's in each layer.
// Avoid any order dependency within a layer.

// Init runs once when the program starts, after the terminal starts.
// All init functions must be independent, and run in alphabetic order.
void init()
{
	PerfTimer perf0("game init");

	BertieBotts::init();
	Creature::init();
	Draw::init();
	Gingerbread::init();
	Item::init();
	LineCache::init();
	Menu::init();
	Random::init();
	Spell::init();
	Status::init();
	Target::init();
}

// Clear runs before the start of each game.
// All clear functions must be independent, and run in alphabetic order.
void clear()
{
	PerfTimer perf0("game clear");

	Bot::clear();
	Creature::clear();
	Draw::clear();
	Gingerbread::clear();
	Input::clear();
	Inventory::clear();
	Item::clear();
	Menu::clear();
	Player::clear();
	Spawn::clear();
	Target::clear();
	World::clear();

	s_filename.clear();
}

void serialize_all(ISerializer& s)
{
	int version = c_VersionNumber;
	s.srz_int(version);
	if (s.is_load() && version != c_VersionNumber)
	{
		// TODO some better in-game handling.
		std::cerr << "Version mismatch.  Cannot load.";
		return;
	}

	s.srz_int(s_turn_number);

	Bot::serialize(s);
	Creature::serialize(s);
	Gingerbread::serialize(s);
	Inventory::serialize(s);
	Item::serialize(s);
	Player::serialize(s);
	Spawn::serialize(s);
	World::edit().serialize(s);

	if (s.is_load())
	{
		// A few things we just shrug and reset.
		Draw::clear();
		Input::clear();
		Menu::clear();
		Target::clear();

		s_game_mode = GameMode::Normal;
		Draw::add_message("Welcome back.");
	}
}

// Setup runs at the start of each game, after character creation.
// Here the ordering may be significant as dependencies appear.
void setup()
{
	PerfTimer perf0("game setup");

	s_turn_number = -1;

	s_game_mode = GameMode::Normal;

	World& world = World::edit();

	Box2 const map1_box = Box2(0, 0, 30, 30);
	int const map1_id = world.add_map(0, 0.0f, map1_box, Terrain::Wall);
	MapGenerator& gen1 = world.edit_map(0).get_generator();
	gen1.Generate();

	// Now try to find a place for the player.
	for (BoxItr itr(world.read_map(0).get_box()); itr; ++itr)
	{
		Vec3 pos = itr->xy0();
		if (world.get_terrain(pos) == Terrain::Open)
		{
			assert(Player::handle().valid());
			Player::handle().move(pos);
			break;
		}
	}

	// Now let's create a number of additional levels on top!
	int constexpr c_NumLevels = 6;
	for (int z = 1; z <= c_NumLevels; ++z)
	{
		int const map_id = world.add_map(z, (z * 0.5f), map1_box, Terrain::Wall);

		MapGenerator::Parameters param{};
		if (z == c_NumLevels)
		{
			param.UpStairsToAdd = 0;
		}
		else
		{
			param.UpStairsToAdd = Random::in_range(2,3);
		}

		MapGenerator& generator = world.edit_map(map_id).get_generator();
		generator.SetParameters(param);
		generator.AddConnectingStairsAsSeedRooms(world.read_map(map_id - 1));
		generator.Generate();

		// Post-process to remove failed stairs from previous level.
		// This is very inelegant (and leaves invalid rooms in the MapGenerator metadata)
		// but a better solution would be more difficult to implement.
		for (Stairs::Pair pair : generator.GetFailedStairs())
		{
			Vec2 other_end = pair.first + Stairs::relative_move(pair.second).xy();
			world.edit_map(map_id - 1).remove_stairs(other_end);
		}
	}

	world.update_visibility(Player::pos(), Player::vision_radius);

	Spawn::post_world_setup();
	Spawn::check_spawning();

	Draw::add_message("Welcome to TROLL.  Press h to see controls.");
	++s_turn_number; // Advance to turn 0

	create_savefile();
}

void update()
{
	if (s_game_mode == GameMode::Normal)
	{
		World::edit().update_visibility(Player::pos(), Player::vision_radius);
		Creature::update_visible_creatures();
		Target::update();
	}

	Draw::draw_screen();

	if (Player::is_automoving() && !terminal_has_input())
	{
		Input::dispatch_automove();
	}
	else
	{
		Input::handle_next_input();
	}

	if (Player::has_acted())
	{
		end_turn();
	}
}

void reset()
{
	clear();
	Menu::show_title();
}

void save()
{
	if (!s_filename.empty())
	{
		SaveGame(s_filename);
	}
}

void load(std::string filename)
{
	s_filename = filename;
	if (!s_filename.empty())
	{
		LoadGame(s_filename);
	}
}

GameMode get_mode()
{
	return s_game_mode;
}

void set_mode(GameMode mode)
{
	s_game_mode = mode;
}

int get_turn_number()
{
	return s_turn_number;
}

//------------------------------------------------------------------------------
// Helper function implementations.

void end_turn()
{
	Player::set_acted(false);

	Status::do_endround(Player::handle());
	Creature::remove_defeated_creatures();
	Spawn::check_spawning();

	// Need to update visibility since player has likely moved.
	World::edit().update_visibility(Player::pos(), Player::vision_radius);
	Creature::update_visible_creatures();

	// Now all other creatures act.
	for (Creature::HandleItr itr(1);
		itr && !Player::is_game_over();
		++itr)
	{
		Bot::do_turn(*itr);
		Status::do_endround(*itr);
		Creature::remove_defeated_creatures();
	}

	// Update clouds
	World::edit().step_clouds();
	Creature::remove_defeated_creatures();

	if (Player::is_game_over())
	{
		game_over();
		return;
	}

	++s_turn_number;
}

void game_over()
{
	// Now it's a roguelike!
	delete_savefile();

	Menu::show_game_over();
}

void create_savefile()
{
	if (!std::filesystem::exists("Save/"))
	{
		std::filesystem::create_directory("Save/");
	}

	s_filename = std::format("Save/{}.sav", Player::name());

	int attempt = 0;
	while (std::filesystem::exists(s_filename) // file already exists
		&& attempt < INT_MAX)
	{
		s_filename = std::format("Save/{}_{}.sav",
			Player::name(), attempt);
		++attempt;
	}

	// Make initial save.
	save();
}

void delete_savefile()
{
	// Let's try not to delete some other file by mistake.
	if (!s_filename.empty() &&
		s_filename.substr(0,5) == "Save/" &&
		s_filename.substr(s_filename.size()-4,4) == ".sav" &&
		std::filesystem::exists(s_filename))
	{
		std::filesystem::remove(s_filename);
		s_filename.clear();
	}
}

} // namespace Game
