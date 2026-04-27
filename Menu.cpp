#include "Menu.h"

#include "BearLibTerminal.h"

#include "Debug.h"
#include "Draw.h"
#include "Input.h"
#include "Inventory.h"
#include "MenuDocument.h"
#include "MenuHighScores.h"
#include "MenuHelp.h"
#include "MenuInventory.h"
#include "MenuLoad.h"
#include "MenuMessages.h"
#include "MenuName.h"
#include "MenuPause.h"
#include "MenuPrologue.h"
#include "MenuSelectHouse.h"
#include "MenuSettings.h"
#include "MenuShopBuy.h"
#include "MenuSpells.h"
#include "MenuTitle.h"
#include "VectorUtil.h"

#if _DEBUG
	#include "MenuDebug.h"
#endif

namespace Menu
{

//-------------------------------------------------------------------------------------------------
// Data

// Menus in alphabetic order
MenuDocument s_menu_document;
MenuHighScores s_menu_high_scores;
MenuHelp s_menu_help;
MenuInventory s_menu_inventory;
MenuLoad s_menu_load;
MenuMessages s_menu_messages;
MenuName s_menu_name;
MenuPrologue s_menu_prologue;
MenuSelectHouse s_menu_select_house;
MenuSettings s_menu_settings;
MenuShopBuy s_menu_shop_buy;
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
	s_menu_help.init();
	s_menu_select_house.init();
	s_menu_title.init();
	s_menu_settings.init();

#if _DEBUG
	s_menu_debug.init();
	s_menu_debug_log_categories.init();
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
	if (s_current_menu != nullptr)
	{
		s_back_stack.push_back(s_current_menu);
	}
}

void update_screen()
{
	if (s_current_menu)
	{
		s_current_menu->draw_screen();
	}
}

Input::Result handle_input(int key)
{
	if (s_current_menu)
	{
		return s_current_menu->handle_input(key);
	}
	else
	{
		DebugBreak("Menu mode with no current menu!");
		return Input::Result::Skipped;
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

void show_document(std::string&& message)
{
	push(); // TODO Maybe this should be automatic for other menus too?
	set_menu(s_menu_document);
	s_menu_document.init(std::move(message));
}

void show_help()
{
	set_menu(s_menu_help);
	//s_menu_help.reset_cursor();
}

void show_game_over()
{
	set_menu(s_menu_high_scores);
	s_menu_high_scores.show_game_over();
}

void show_high_scores()
{
	set_menu(s_menu_high_scores);
	s_menu_high_scores.show_scores();
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

void show_prologue()
{
	set_menu(s_menu_prologue);
	s_menu_prologue.refresh();
}

void show_spells_known()
{
	set_menu(s_menu_spells);
	s_menu_spells.show_known_spells();
	s_menu_load.reset_cursor();
}

void show_inventory()
{	
	if (Inventory::read().num_slots() > 0)
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

void show_shop_buy()
{
	set_menu(s_menu_shop_buy);
	s_menu_shop_buy.refresh();
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
	//s_menu_debug_log_categories.refresh();
	s_menu_debug_log_categories.reset_cursor();
}
#endif

} // namespace Menu
