#include "Game.h"

#include "Bot.h"
#include "Draw.h"
#include "Input.h"
#include "Line.h"
#include "Map.h"
#include "MapGenerator.h"
#include "Menu.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "Stairs.h"
#include "Status.h"
#include "Target.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

namespace Game
{

int s_turn_number;
GameMode s_game_mode;

std::vector<bool> s_spawned;

//------------------------------------------------------------------------------
// Helper function declarations.
void end_turn();
void game_over();
void check_spawning();

//------------------------------------------------------------------------------
// Interface function implementations

// Initialization is in several layers (init, clear, setup).
// Alphabetize the init's in each layer.
// Avoid any order dependency within a layer.

// Init runs once when the program starts.
void init()
{
	Creature::init();
	Draw::init();
	LineCache::init();
	Random::init();
	Spell::init();
	Status::init();
}

// Clear runs before the start of each game.
void clear()
{
	Creature::clear();
	Draw::clear();
	Input::clear();
	Player::clear();
	Target::clear();
	World::clear();
}

// Setup runs at the start of each game, after all clear functions.
void setup()
{
	s_turn_number = 0;
	s_game_mode = GameMode::Normal;
	s_spawned.clear();
	s_spawned.reserve(7);

	World& world = World::edit();

	Box2 const map1_box = Box2(0, 0, 30, 30);
	int const map1_id = world.add_map(0, map1_box, Terrain::Wall);
	MapGenerator& gen1 = world.edit_map(0).get_generator();
	gen1.Generate();
	s_spawned.push_back(false);

	// Now try to find a place for the player...
	for (BoxItr itr(world.read_map(0).get_box()); itr; ++itr)
	{
		Vec3 pos = itr->xy0();
		if (world.get_terrain(pos) == Terrain::Open)
		{
			spawn_creature(Creature::Player, pos);
			break;
		}
	}

	// Now let's create a number of additional levels on top!
	int constexpr c_num_levels = 6;
	for (int z = 1; z <= c_num_levels; ++z)
	{
		int const map_id = world.add_map(z, map1_box, Terrain::Wall);

		MapGenerator::Parameters param{};
		if (z == c_num_levels)
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

		s_spawned.push_back(false);
	}

	world.update_visibility(Player::pos(), Player::vision_radius);
	check_spawning();

/*	Box2 const map1_box = Box2(0, 0, 24, 24);
	int const map1_id = world.add_map(0, map1_box, Terrain::Wall);
	Box2 const map2_box = Box2(0, -10, 24, 10);
	int const map2_id = world.add_map(0, map2_box, Terrain::Wall);
	int const map3_id = world.add_map(1, map1_box, Terrain::Wall);

	Map& map1 = world.edit_map(map1_id);
	Map& map2 = world.edit_map(map2_id);
	Map& map3 = world.edit_map(map3_id);

	map1.fill_box(Box2(4, 4, 15, 15), Terrain::Open);
	map1.fill_box(Box2(7, 5, 4, 1), Terrain::Wall);
	map1.fill_box(Box2(14, 8, 1, 5), Terrain::Wall);
	map1.fill_box(Box2(4, 0, 1, 5), Terrain::Open);
	map1.add_stairs({6,3}, Stairs::UpNorth);

	map2.fill_box(Box2(1, -9, 9, 9), Terrain::Open);
	map2.set_terrain({4,-1}, Terrain::Open);

	map3.fill_box(Box2(1,1,12,1), Terrain::Open);
	map3.add_corresponding_stairs(map1);

	spawn_creature(Creature::Player, {4,4});
	spawn_creature(Creature::Neville_0, {6,9});
	spawn_creature(Creature::ColinCreevy_0, {13,15});*/
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

	if (Player::is_automoving())
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
	setup();
	Menu::show_title();

	--s_turn_number;
	Draw::add_message("Welcome to TROLL.  Press h to see controls.");
	++s_turn_number;
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

	// Now all other creatures act.
	for (Creature::HandleItr itr(1);
		itr && !Player::is_game_over();
		++itr)
	{
		Bot::do_turn(*itr);
		Status::do_endround(*itr);
		Creature::remove_defeated_creatures();
	}

	if (Player::is_game_over())
	{
		game_over();
		return;
	}

	check_spawning();

	++s_turn_number;
}

void game_over()
{
	Menu::show_game_over();
}

void check_spawning()
{
	// TODO There's probably a better place to put this code.
	int const map_id = World::read().find_map(Player::pos());

	if (Util::IsValidIndex(s_spawned, map_id) && !s_spawned[map_id])
	{
		Map const& map = World::read().read_map(map_id);
		std::cout << "\nSpawning for map " << map_id << ".\n";

		int const num_to_spawn = Random::in_range(4,6);
		int num_spawned = 0;
		for (int i = 0; i < num_to_spawn; ++i)
		{
			// Find spawn position
			// TODO There's definitely a better way to do this.
			// Like get the list of rooms, etc.
			int const attempts = 100;
			for (int a = 0; a < attempts; ++a)
			{
				Vec2 const pos2 = Random::in_box(map.get_box());
				Vec3 const pos3 = pos2.xyz(Player::pos().z);
				if (map.get_terrain(pos2) == Terrain::Open &&
					!World::read().is_visible(pos3) &&
					Creature::creature_at_pos(pos3) == Creature::None)
				{
					// TODO difficulty per map
					Creature::Type type = Creature::find_type_to_spawn(1);
					if (type != Creature::None)
					{
						Creature::Handle creature = Creature::spawn_creature(type, pos3);
						std::cout << " - Spawned " << creature.long_name()
							<< " at " << creature.pos().x << ", " << creature.pos().y << ".\n";
						++num_spawned;
					}
					break;
				}
			}
		}

		std::cout << "Spawned " << num_spawned << "/" << num_to_spawn
			<< " for map " << map_id << ".\n";
		s_spawned[map_id] = true;
	}
}

}
