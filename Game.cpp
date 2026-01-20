#include "Game.h"

#include "Bot.h"
#include "Draw.h"
#include "Input.h"
#include "Player.h"
#include "Map.h"
#include "Menu.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"

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
}

// Setup runs at the start of each game, after all clear functions.
void setup()
{
	s_turn_number = 0;
	s_game_mode = GameMode::Normal;

	Map& map = g_map();
	Box map_box = make_box(0, 0, 100, 100);
	map.init(map_box, Terrain::Wall);
	map.fill_box(make_box(4, 4, 15, 15), Terrain::Open);
	map.fill_box(make_box(7, 5, 4, 1), Terrain::Wall);
	map.clear_visibility();

	spawn_creature(Creature::Player, { 4,4 });
	spawn_creature(Creature::Neville_0, { 6,9 });
	spawn_creature(Creature::ColinCreevy_0, { 14,15 });
	//	spawn_creature(Creature::ColinCreevy_0, {14,17});
	//	spawn_creature(Creature::ColinCreevy_0, {16,15});
	//	spawn_creature(Creature::ColinCreevy_0, {15,16});
	//	spawn_creature(Creature::ColinCreevy_0, {8,13});
}

void update()
{
	Player& player = g_player();
	Map& map = g_map();

	if (s_game_mode == GameMode::Normal)
	{
		map.update_visibility(Player::pos(), player.vision_radius);
		Creature::update_visible_creatures();
		Target::update();
	}

	Draw::draw_screen();
	Input::handle_next_input();

	if (player.acted)
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
	g_player().acted = false;

	Status::do_endround(Player::handle());
	Creature::remove_defeated_creatures();

	// Now all other creatures act.
	for (Creature::HandleItr itr(1);
		itr && !g_player().game_over;
		++itr)
	{
		Bot::do_turn(*itr);
		Status::do_endround(*itr);
		Creature::remove_defeated_creatures();
	}

	if (g_player().game_over)
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
