#pragma once

#include "Types.h"

namespace Menu
{
	enum Type : int
	{
		None,
		Document,
		Count
	};

	// Go to menu mode and open a specific menu.
	void show_help();

	// Return to normal gameplay
	void close();

	void update_screen();
	void handle_input(int key);
}