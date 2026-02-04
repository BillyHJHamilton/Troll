#include "BearLibTerminal.h"

#include "Geometry.h"
#include "Game.h"
#include "Input.h"

// To do:
// - Resting to regain HP
// - Name and difficulty level for each map
// - Improve spawning code
//  - Find pos by room, instead of spraying at random
// - Improved enemy AI
//  - Pathfinding
//  - EQS / find good attack position
// - XP and level up
// - Learning spells
// - Items
// - Proper miscasts
// - NPC taunts

// Bugs:
// - Targeting can get stuck on the other level
//  - Should probably cache the view and use this to help targeting

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
