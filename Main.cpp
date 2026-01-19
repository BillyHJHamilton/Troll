#include "BearLibTerminal.h"

#include "Creature.h"
#include "Config.h"
#include "Draw.h"
#include "Input.h"
#include "Geometry.h"
#include "Game.h"
#include "Global.h"
#include "Map.h"
#include "Menu.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"

// To do:
// - Improved enemy AI
//  - Take a turn to wake up and see player.
//  - Move towards player's last location if out of range.
//  - Pathfinding...
//  - EQS / find good attack position
// - XP and level up
// - Map generation
// - Improved turn and action timing

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

int main()
{
    terminal_open();
  
	config_terminal();

	Game::init();

	Game::reset();

	while (!g_quit_flag)
	{
		Game::update();
	}
  
    terminal_close();

	return 0;
}