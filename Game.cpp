#include "Game.h"

#include "Bot.h"
#include "Draw.h"
#include "Global.h"
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
	init_draw();
	init_random();
	Spell::init();
	Status::init();
}

// Clear runs before the start of each game.
void clear()
{
	Creature::clear();
	clear_input();
	Player::clear();
	clear_target();
}

// Setup runs at the start of each game, after all clear functions.
void setup()
{
	setup_global();
}

void update()
{
	Player& player = g_player();
	Map& map = g_map();

	if (g_game_mode == GameMode::Normal)
	{
		map.update_visibility(Player::pos(), player.vision_radius);
		Creature::update_visible_creatures();
		update_target();
	}

	draw_screen();
	handle_next_input();

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
}

//------------------------------------------------------------------------------
// Helper function implementations.

void end_turn()
{
	g_player().acted = false;

	Bot::do_all_bot_turns();

	for (Creature::HandleItr itr(0); itr; ++itr)
	{
		Status::do_endround(*itr);
	}

	if (Player::handle().hp() <= 0)
	{
		game_over();
		return;
	}

	Creature::remove_defeated_creatures();
}

void game_over()
{
	Menu::show_game_over();
}

}
