#include "BearLibTerminal.h"

#include "Creature.h"
#include "Config.h"
#include "Draw.h"
#include "Input.h"
#include "Geometry.h"
#include "Global.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"

// To do:
// X Map
// X Draw screen
// X Visibility (algorithm could be improved)
// X Basic movement
// X Not walk through walls
// X Screen layout
// X Non-player character
// X Targeting
// X Spell parsing
// X Spell casting
// X Spell effects
// X Message printing
// X Dynamic stat display
// X Status effects
// X Spell accuracy/failure
// X Spell animations
// - Manual targeting
// - AI firing back
// - Turn and action timing
// - Check spells known
// - Map generation

void test_line_drawing()
{
	for (int y = 0; y <= 20; y += 2)
	{
		for (LineItr itr({0,10}, {10,y}); !itr.finished(); ++itr)
		{
			terminal_put(itr->x, itr->y, 'a'+y/2);
		}
	}
}

void game_loop()
{
	Player & player = g_player();
	Map & map = g_map();

	map.update_visibility(Player::pos(), player.vision_radius);
	update_visible_creatures();
	update_target();

	draw_screen();

	handle_next_input();
}

int main()
{
    terminal_open();
  
	config_terminal();

	// Initialization is in several layers (init, clear, setup).
	// Alphabetize the init's in each layer.
	// Avoid any order dependency within a layer.

	// Init functions run once when the program starts
	init_creatures();
	init_draw();
	init_random();
	Spell::init();
	Status::init();

	// Clear functions run before start of each game
	clear_creatures();
	clear_input();
	clear_target();

	// Setup functions run at the start of each game, after all clear functions
	setup_global();

	while (!g_quit_flag)
	{
		game_loop();
	}
  
    terminal_close();

	return 0;
}