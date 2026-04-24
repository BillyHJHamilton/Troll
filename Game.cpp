#include "Game.h"

#include "Ability.h"
#include "BertieBotts.h"
#include "Bot.h"
#include "BuildWorld.h"
#include "Config.h"
#include "Confirm.h"
#include "Crosshair.h"
#include "Debug.h"
#include "Draw.h"
#include "Feature.h"
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
#include "Scratch.h"
#include "Score.h"
#include "SerializeSaveLoad.h"
#include "Spawn.h"
#include "Spell.h"
#include "Stairs.h"
#include "Status.h"
#include "Squad.h"
#include "Taunt.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <filesystem>

namespace Game
{

int constexpr c_AutosaveFrequency = 25;

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

// Init runs once when the program starts, after the terminal opens.
// All init functions must be independent, and run in alphabetic order.
void init()
{
	PerfTimer perf0("game init");

	Ability::init();
	BertieBotts::init();
	Creature::init();
	Debug::init();
	Draw::init();
	Feature::init();
	Gingerbread::init();
	Item::init();
	LineCache::init();
	Menu::init();
	Random::init();
	Score::init();
	Status::init();
	Squad::init();
	Taunt::init();

	Config::load();
}

// Clear runs before the start of each game.
// All clear functions must be independent, and run in alphabetic order.
void clear()
{
	PerfTimer perf0("game clear");

	Ability::clear();
	Bot::clear();
	Creature::clear();
	Crosshair::clear();
	Draw::clear();
	Feature::clear();
	Gingerbread::clear();
	Input::clear();
	Inventory::clear();
	Item::clear();
	Menu::clear();
	Player::clear();
	Score::clear();
	Spawn::clear();
	Squad::clear();
	Taunt::clear();
	World::clear();

	s_filename.clear();
}

//-----------------------------------------------------------------------------
// Serialization

void VersionNumber::serialize(ISerializer& s)
{
	s.srz_int(major);
	s.srz_int(minor);
	s.srz_int(patch);
}

bool VersionNumber::can_load() const
{
	return major == c_MajorVersion
		&& minor == c_MinorVersion
		&& patch <= c_PatchVersion;
}

bool serialize_file_type_label(ISerializer& s)
{
	char label [10] = "TROLLGAME";
	static std::string const c_Label(label);

	s.srz_array_data(label, 9); // don't serialize the null terminator
	return (c_Label == label);
}

bool try_serialize_all(ISerializer& s)
{
	PerfTimer perf("try_serialize_all");

	bool file_valid = serialize_file_type_label(s);
	if (!file_valid)
	{
		Menu::show_document("Loading failed - invalid file format.\n\n(Press enter)");
		return false;
	}

	VersionNumber version;
	version.serialize(s);
	if (!version.can_load())
	{
		Menu::show_document("Loading failed - invalid version number.\n\n(Press enter)");
		return false;
	}

	// Serialize the player and the turn number first, to display when loading.
	Player::serialize(s);
	s.srz_int(s_turn_number);

	// Then serialize everything else in alphabetical order.
	Ability::serialize(s);
	Bot::serialize(s);
	Creature::serialize(s);
	Feature::serialize(s);
	Gingerbread::serialize(s);
	Inventory::serialize(s);
	Item::serialize(s);
	Spawn::serialize(s);
	Squad::serialize(s);
	Taunt::serialize(s);
	World::edit().serialize(s);

	if (s.is_load())
	{
		// A few things we just shrug and reset.
		Crosshair::clear();
		Draw::clear();
		Input::clear();
		Menu::clear();

		s_game_mode = GameMode::Normal;
		Draw::add_message("Welcome back.");
	}

	return true;
}

// Setup runs at the start of each game, after character creation.
// Here the ordering may be significant as dependencies appear.
void setup()
{
	PerfTimer perf0("game setup");

	s_turn_number = -1;

	s_game_mode = GameMode::Normal;

	BuildWorld();

	// Now find a place for the player.
	assert(Player::handle().valid());
	Player::handle().move(World::read().get_player_start());

	World::edit().update_visibility(Player::pos(), Player::c_VisionRadius);

	Spawn::check_spawning();

	Draw::add_message("Welcome to TROLL.  Press H for help.");

	++s_turn_number; // Advance to turn 0

	create_savefile();
}

void update()
{
	// Confirm that temp memory from last frame was released.
	assert(Scratchpad::is_empty());

	if (s_game_mode == GameMode::Normal)
	{
		World::edit().update_visibility(Player::pos(), Player::c_VisionRadius);
		Creature::update_visible_creatures();
		Crosshair::update();
	}

	Draw::draw_screen();

	bool did_something = false;
	while (!did_something)
	{
		if (terminal_has_input() || !Player::is_automoving())
		{
			Input::Result result = Input::handle_next_input();
			if (result == Input::Result::Skipped)
			{
				continue;
			}
			else if (result == Input::Result::Handled)
			{
				did_something = true;
				Player::stop_automove();
			}
			else if (result == Input::Result::StartAutomate)
			{
				did_something = true;
			}
		}

		if (Player::is_automoving())
		{
			Player::dispatch_automove();
			did_something = true;
		}
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
		SaveSerializer s(s_filename);
		try_serialize_all(s);
	}
}

void load(std::string filename)
{
	if (!filename.empty())
	{
		LoadSerializer s(filename);
		bool loaded = try_serialize_all(s);
		if (loaded)
		{
			s_filename = filename;
		}
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
	PerfTimer perf("Game::end_turn");

	Player::set_acted(false);

	Player::handle().endround();
	Creature::remove_defeated_creatures();
	Spawn::check_spawning();

	// Need to update visibility since player has likely moved.
	World::edit().update_visibility(Player::pos(), Player::c_VisionRadius);
	Feature::update_all();  // uses visibility, but can change it
	World::edit().update_visibility(Player::pos(), Player::c_VisionRadius);
	Creature::update_visible_creatures();

	// Now all other creatures act.
	for (Creature::HandleItr itr(1);
		itr && !Player::is_game_over();
		++itr)
	{
		Bot::do_turn(*itr);
		Creature::remove_defeated_creatures(); // Check often so message order feels right...
		if (itr->valid() && !Player::is_game_over())
		{
			itr->endround();
			Creature::remove_defeated_creatures();
		}
	}

	// Update clouds
	if (!Player::is_game_over())
	{
		World::edit().step_clouds();
		Creature::remove_defeated_creatures();
	}

	Ability::tick_cooldowns();

	if (Player::is_game_over())
	{
		Player::stop_automove();
		Confirm::press_enter(&Game::game_over);
		return;
	}

	if (s_turn_number % c_AutosaveFrequency == 0)
	{
		Game::save();
	}

	Player::tick_sugar();

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
