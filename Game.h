#pragma once

// Game.cpp includes almost everything; but Game.h is widely needed, so keep it light.

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
	void clear();	// Runs before the start of each game.
	void setup();	// Runs at the start of each game, after clear.

	// Redraw the screen and process input.
	void update();

	// Call to restart the game and return to the beginning.
	void reset();

	GameMode get_mode();
	void set_mode(GameMode mode);
	int get_turn_number();

	World& get_world();
}
