#pragma once

#include "Types.h"
#include <string>

class World;

enum class GameMode : byte
{
	Normal,
	Menu
};

namespace Game
{
	// Initialization is in several layers:
	void init();	// Runs only once, when the program starts.
	void clear();	// Runs at the start of each game, before the main menu.
	void setup();	// Runs at the end of character creation, to start the game.

	// Serializes the entire game state.
	void serialize_all(ISerializer& s);

	// Redraw the screen and process input.
	void update();

	// Clears everything and returns to the main menu.
	void reset();

	void save();
	void load(std::string filename);

	GameMode get_mode();
	void set_mode(GameMode mode);
	int get_turn_number();
}
