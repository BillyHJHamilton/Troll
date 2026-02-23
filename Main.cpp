#include "BearLibTerminal.h"

#include "Config.h"
#include "Geometry.h"
#include "Game.h"
#include "Input.h"
#include "PerfTimer.h"

void config_terminal()
{
	terminal_set("window.size=120x31");
	terminal_set("window.title='TROLL'");

	// TODO Load config file, apply config.

	Config::load_fonts(false);

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
  
#if d_PerfTest
	PerfTimer::PrintAll();
#endif

    terminal_close();

	return 0;
}
