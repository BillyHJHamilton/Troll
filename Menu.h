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

	// Document Menus
	void show_title();
	void show_help();
	void show_game_over();

	// List Menus
	void show_house_selection();
	void show_starting_spells();
	void show_spells_known();
	void show_inventory();

	// Return to normal gameplay
	void close();

	void update_screen();
	void handle_input(int key);
}