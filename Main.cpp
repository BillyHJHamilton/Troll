#include "BearLibTerminal.h"

#include "Config.h"
#include "Geometry.h"
#include "Game.h"
#include "Global.h"

// To do:
// - Improved enemy AI
//  - Move towards player's last location if out of range.
//  - Pathfinding...
//  - EQS / find good attack position
// - XP and level up
// - Map generation
// - Multiple floors / levels
// - Items

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
