#include "Game.h"

#include "Bot.h"
#include "Draw.h"
#include "Input.h"
#include "Line.h"
#include "Map.h"
#include "Menu.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"
#include "Terrain.h"
#include "World.h"

namespace Game
{

int s_turn_number;
GameMode s_game_mode;

//------------------------------------------------------------------------------
// Helper function declarations.
void end_turn();
void game_over();

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
	World& world = World::edit();

	Box2 const map1_box = Box2(0, 0, 24, 24);
	int const map1_id = world.add_map(0, map1_box, Terrain::Wall);
	Box2 const map2_box = Box2(0, -10, 24, 10);
	int const map2_id = world.add_map(0, map2_box, Terrain::Wall);

	Map& map1 = world.edit_map(map1_id);
	Map& map2 = world.edit_map(map2_id);

	map1.fill_box(Box2(4, 4, 15, 15), Terrain::Open);
	map1.fill_box(Box2(7, 5, 4, 1), Terrain::Wall);
	map1.fill_box(Box2(14, 8, 1, 5), Terrain::Wall);
	map1.fill_box(Box2(4, 0, 1, 5), Terrain::Open);

	map2.fill_box(Box2(1, -9, 9, 9), Terrain::Open);
	map2.set_terrain({4,-1}, Terrain::Open);

	spawn_creature(Creature::Player, {4,4});
	spawn_creature(Creature::Neville_0, {6,9});
	spawn_creature(Creature::ColinCreevy_0, {13,15});
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
	Input::handle_next_input();

	if (Player::data().acted)
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
		itr && !Player::data().game_over;
		++itr)
	{
		Bot::do_turn(*itr);
		Status::do_endround(*itr);
		Creature::remove_defeated_creatures();
	}

	if (Player::data().game_over)
	{
		game_over();
		return;
	}

	++s_turn_number;
}

void game_over()
{
	Menu::show_game_over();
}

}
