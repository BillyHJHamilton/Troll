#include "MenuPause.h"
#include "Input.h"
#include "BearLibTerminal.h"

MenuPause::MenuPause()
{
	m_title = "Pause Menu:";
	m_options = 
	{
		{"Resume", PauseMenuOption::Resume},
		{"Spells Known", PauseMenuOption::SpellsKnown},
		{"Inventory", PauseMenuOption::Inventory},
		{"Help", PauseMenuOption::Help},
		{"Message History", PauseMenuOption::MessageHistory},
		{"Quit", PauseMenuOption::Quit},
	};
}

void MenuPause::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		switch(get_selected().value)
		{
			case PauseMenuOption::Resume:
				Menu::close();
				break;
			case PauseMenuOption::SpellsKnown:
				Menu::show_spells_known();
				break;
			case PauseMenuOption::Inventory:
				Menu::show_inventory();
				break;
			case PauseMenuOption::Help:
				Menu::show_help();
				break;
			case PauseMenuOption::MessageHistory:
				Menu::show_message_history();
				break;
			case PauseMenuOption::Quit:
				Input::request_quit();
				break;
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}
