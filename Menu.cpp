#include "Menu.h"

#include "BearLibTerminal.h"

#include "Draw.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Input.h"
#include "Inventory.h"
#include "MenuDocument.h"
#include "MenuInventory.h"
#include "MenuLoad.h"
#include "MenuMessages.h"
#include "MenuName.h"
#include "MenuPause.h"
#include "MenuSelectHouse.h"
#include "MenuSettings.h"
#include "MenuSpells.h"
#include "MenuTitle.h"
#include "Player.h"
#include "VectorUtil.h"

#include <format>

#if _DEBUG
	#include "MenuDebug.h"
#endif

namespace Menu
{

//-------------------------------------------------------------------------------------------------
// Data

// Menus in alphabetic order
MenuDocument s_menu_document;
MenuInventory s_menu_inventory;
MenuLoad s_menu_load;
MenuMessages s_menu_messages;
MenuName s_menu_name;
MenuSelectHouse s_menu_select_house;
MenuSettings s_menu_settings;
MenuSpells s_menu_spells;
MenuTitle s_menu_title;
MenuPause s_menu_pause;

// Debug menus
#if _DEBUG
	MenuDebug s_menu_debug;
	MenuDebugLogCategories s_menu_debug_log_categories;
#endif

IMenu* s_current_menu = nullptr;
std::vector<IMenu*> s_back_stack;

//-------------------------------------------------------------------------------------------------
// Static Data

const char* const cstr_DocHelp =
	"How To Play\n"
	"\n"
	"Movement:\n"
	"  To move, use the arrow keys or numpad.\n"
	"  If you have no numpad, use Home/End/PgUp/PgDn for diagonals.\n"
	"  For a long move, hold Ctrl and press the move key.\n"
	"\n"
	"Resting:\n"
	"  To skip a turn, press Space (or numpad 5).\n"
	"  To rest until fully healed, hold Ctrl and press Space (or numpad 5).\n"
	"\n"
	"Spellcasting:\n"
	"  To cast a spell, hold Shift and type the spell's two-letter abbreviation.\n"
	"  To see your spells, press ? (shift /).\n"
	"\n"
	"Spell Targeting:\n"
	"  Your current target is highlighted on the map.\n"
	"  To cycle between targets, press Tab.\n"
	"  To target a square manually, hold shift and use the move controls.\n"
	"\n"
	"Items:\n"
	"  To collect items, simply walk onto them.\n"
	"  To view your inventory, press 'i'.\n"
	"  To use an item, select it in the inventory and press Enter.\n"
	"\n"
	"To show these instructions again, press 'h'.\n";

//-------------------------------------------------------------------------------------------------
// Helper function

void set_menu(IMenu& menu)
{
	Game::set_mode(GameMode::Menu);
	s_current_menu = &menu;
}

//-------------------------------------------------------------------------------------------------
// Public function implementations

void init()
{
	s_menu_select_house.init();
	s_menu_title.init();
	s_menu_settings.init();

#if _DEBUG
	s_menu_debug.init();
#endif

	s_back_stack.reserve(4);
}

void clear()
{
	s_current_menu = nullptr;
	s_back_stack.clear();
}

void close()
{
	s_current_menu = nullptr;
	s_back_stack.clear();
	Game::set_mode(GameMode::Normal);
	Input::clear();
}

void back()
{
	if (s_back_stack.empty())
	{
		close();
	}
	else
	{
		s_current_menu = Util::PopBack(s_back_stack);
	}
}

void push()
{
	s_back_stack.push_back(s_current_menu);
}

void update_screen()
{
	if (s_current_menu)
	{
		s_current_menu->draw_screen();
	}
}

void handle_input(int key)
{
	if (s_current_menu)
	{
		s_current_menu->handle_input(key);
	}
}

//-------------------------------------------------------------------------------------------------
// List of menus

void show_title()
{
	set_menu(s_menu_title);
	s_menu_title.reset_cursor();
}

void show_load()
{
	set_menu(s_menu_load);
	s_menu_load.refresh();
}

void show_name_entry()
{
	set_menu(s_menu_name);
	s_menu_name.init();
}

void show_help()
{
	set_menu(s_menu_document);
	s_menu_document.init(cstr_DocHelp);
}

void show_game_over()
{
	set_menu(s_menu_document);

	std::string content = std::format(
		"Game Over.\n\n"
		"You were defeated by {}.",
		Gingerbread::long_name(Player::get_defeated_by()));

	s_menu_document.init(content, &Game::reset);
}

void show_house_selection()
{
	set_menu(s_menu_select_house);
	s_menu_select_house.reset_cursor();
}

void show_starting_spells()
{
	set_menu(s_menu_spells);
	s_menu_spells.show_starting_spells();
	s_menu_load.reset_cursor();
}

void show_spells_known()
{
	set_menu(s_menu_spells);
	s_menu_spells.show_known_spells();
	s_menu_load.reset_cursor();
}

void show_inventory()
{	
	if (Inventory::read().num_items() > 0)
	{
		set_menu(s_menu_inventory);
		s_menu_inventory.refresh();
		s_menu_inventory.reset_cursor();
	}
	else
	{
		Menu::close();
		Draw::add_message("Your inventory is empty.");
		return;
	}
}

void show_pause_menu()
{
	set_menu(s_menu_pause);
	s_menu_pause.reset_cursor();
}

void show_message_history()
{
	set_menu(s_menu_messages);
	s_menu_messages.init();
	s_menu_messages.scroll_to_end();
}

void show_settings()
{
	set_menu(s_menu_settings);
	s_menu_settings.reset_cursor();
}

#if _DEBUG
void show_debug_menu()
{
	set_menu(s_menu_debug);
	s_menu_debug.reset_cursor();
}

void show_debug_log_categories()
{
	set_menu(s_menu_debug_log_categories);
	s_menu_debug_log_categories.refresh();
	s_menu_debug_log_categories.reset_cursor();
}
#endif

} // namespace Menu
