#pragma once

namespace Game
{
	// Init runs once when the program starts.
	void init();

	// Clear runs before the start of each game.
	void clear();

	// Setup runs at the start of each game, after all clear functions.
	void setup();

	// Redraw the screen and process input.
	void update();

	// Call to restart the game and return to the beginning.
	void reset();
}
