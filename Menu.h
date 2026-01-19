#pragma once

#include "Types.h"

namespace Menu
{
	enum Type : int
	{
		None,
		Document,
		List,
		Count
	};

	void show_title();
	void show_help();
	void show_game_over();
	void show_spells_known();

	// Return to normal gameplay
	void close();

	void update_screen();
	void handle_input(int key);
}