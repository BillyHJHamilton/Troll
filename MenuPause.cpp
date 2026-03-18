#include "MenuPause.h"

#include "Input.h"
#include "Game.h"

#include "BearLibTerminal.h"

MenuPause::MenuPause()
{
	m_title = "In-Game Menu:";
	m_options = 
	{
		{"Resume", PauseMenuOption::Resume},
		{"Spells Known", PauseMenuOption::SpellsKnown},
		{"Inventory", PauseMenuOption::Inventory},
		{"Help", PauseMenuOption::Help},
		{"Message History", PauseMenuOption::MessageHistory},
		{"Settings", PauseMenuOption::Settings},
		{"Save and Quit", PauseMenuOption::SaveAndQuit},
	};
}

Input::Result MenuPause::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		switch(get_selected().value)
		{
			case PauseMenuOption::Resume:
				Menu::close();
				return Input::Result::Handled;

			case PauseMenuOption::SpellsKnown:
				Menu::push();
				Menu::show_spells_known();
				return Input::Result::Handled;

			case PauseMenuOption::Inventory:
				Menu::push();
				Menu::show_inventory();
				return Input::Result::Handled;

			case PauseMenuOption::Help:
				Menu::push();
				Menu::show_help();
				return Input::Result::Handled;

			case PauseMenuOption::MessageHistory:
				Menu::push();
				Menu::show_message_history();
				return Input::Result::Handled;

			case PauseMenuOption::Settings:
				Menu::push();
				Menu::show_settings();
				return Input::Result::Handled;

			case PauseMenuOption::SaveAndQuit:
				Game::save();
				Game::reset();
				return Input::Result::Handled;
		}
	}

	return MenuList::handle_input(key);
}
