#pragma once

#include "Types.h"
#include "Input.h"
#include <string>

// Interface for a UI menu type.
class IMenu
{
public:
	virtual ~IMenu() {}

	virtual void draw_screen () = 0;
	virtual Input::Result handle_input (int key) = 0;
};

namespace Menu
{
	void init();
	void clear();

	// Return to normal gameplay.
	void close();

	// Return to a previous menu, or close if none.
	void back();

	// Remember the current menu (so we can go back later).
	void push();

	void update_screen();
	Input::Result handle_input(int key);

	//-----------------------------------------------------
	// List of menus:

	// Menus at startup:
	void show_title();
	void show_load();
	void show_name_entry();
	void show_house_selection();
	void show_starting_spells();

	// Other Documents
	void show_document(std::string&& message);
	void show_help();
	void show_game_over();
	void show_high_scores();

	// List Menus
	void show_spells_known();
	void show_inventory();
	void show_pause_menu();
	void show_message_history();
	void show_settings();

	// Debug menus
#if _DEBUG
	void show_debug_menu();
	void show_debug_log_categories();
#endif
}
