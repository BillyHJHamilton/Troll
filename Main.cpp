#include "BearLibTerminal.h"

#include "Geometry.h"
#include "Game.h"
#include "Input.h"
#include "PerfTimer.h"

// To do:
// - Items
// - Learning spells
// - Proper miscasts
// - Name and difficulty level for each map
// - Improve spawning code
//  - Find pos by room, instead of spraying at random?
//  - Or enumerate all open spaces and choose some?
// - NPC taunts
// - Improved enemy AI
//  - Wander a little more searching for player.  Maybe travel to random points nearby, or something.
//  - EQS / find good attack position

// To consider:
// - How do we handle initiative to support the "ping pong" shield gameplay

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
  
#if PERF_TEST
	PerfTimer::PrintAll();
#endif

    terminal_close();

	return 0;
}
