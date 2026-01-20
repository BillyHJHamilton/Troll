#include "BearLibTerminal.h"

#include "Geometry.h"
#include "Game.h"
#include "Input.h"

// To do:
// - Grey out messages from previous turns so you can see what's new
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

void config_terminal()
{
	char const* FONT1 = "FSEX302.ttf";
	char const* FONT2 = FONT1;

	terminal_set("window.size=120x31");
	terminal_set("window.title='TROLL'");
	terminal_setf("font: %s, size=8x16", FONT1);
	terminal_setf("tile font: %s, size=16x16, spacing=2x1", FONT2);

	// register for input
	terminal_set
	(
		"input.filter=[keyboard+]"
	);

	//terminal_set("window.cellsize=8x16");
}

int main()
{
    terminal_open();
  
	config_terminal();

	Game::init();

	Game::reset();

	while (!Input::is_quitting())
	{
		Game::update();
	}
  
    terminal_close();

	return 0;
}
