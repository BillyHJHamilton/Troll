#include "config.h"
#include "BearLibTerminal.h"

void config_terminal()
{
	char const * FONT1 = "FSEX302.ttf";
	char const * FONT2 = FONT1;

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
