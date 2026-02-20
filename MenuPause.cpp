#include "MenuPause.h"

#include "Input.h"
#include "Game.h"

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
		//{"Save", PauseMenuOption::Save},
		//{"Load", PauseMenuOption::Load},
		{"Save and Quit", PauseMenuOption::SaveAndQuit},
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
				Menu::push();
				Menu::show_spells_known();
				break;
			case PauseMenuOption::Inventory:
				Menu::push();
				Menu::show_inventory();
				break;
			case PauseMenuOption::Help:
				Menu::push();
				Menu::show_help();
				break;
			case PauseMenuOption::MessageHistory:
				Menu::push();
				Menu::show_message_history();
				break;
			//case PauseMenuOption::Save:
			//	Game::save();
			//	Menu::close();
			//	break;
			//case PauseMenuOption::Load:
			//	Menu::push();
			//	Menu::show_load();
			//	break;
			case PauseMenuOption::SaveAndQuit:
				Game::save();
				Game::reset();
				break;
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}
