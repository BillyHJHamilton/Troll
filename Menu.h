#pragma once

#include "Types.h"

// Interface for a UI menu type.
class IMenu
{
public:
	virtual ~IMenu() {}

	virtual void draw_screen () = 0;
	virtual void handle_input (int key) = 0;
};

namespace Menu
{
	enum Type : int
	{
		None,
		Document,
		List,
		Count
	};

	void clear();

	// Document Menus
	void show_title();
	void show_help();
	void show_game_over();

	// List Menus
	void show_house_selection();
	void show_starting_spells();
	void show_spells_known();
	void show_inventory();
	void show_pause_menu();

	// Return to normal gameplay
	void close();

	void update_screen();
	void handle_input(int key);
}
