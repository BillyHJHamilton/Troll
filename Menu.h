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

	// Go to menu mode and open a specific menu.
	void show_title();
	void show_help();
	void show_spells_known();

	// Return to normal gameplay
	void close();

	void update_screen();
	void handle_input(int key);
}