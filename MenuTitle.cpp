#include "MenuTitle.h"

#include "Input.h"
#include "Game.h"

#include "BearLibTerminal.h"

char const* MenuTitle::cstr_TitleText =
	"\n"
	"  ------------------------------------\n\n"
	"  TTTTTTT RRRR    OOO    L      L     \n"
	"     T    R   R  O   O   L      L     \n"
	"     T    R  R  O     O  L      L     \n"
	"     T    RRR   O     O  L      L     \n"
	"     T    R  R   O   O   L      L     \n"
	"     T    R   R   OOO    LLLLLL LLLLLL\n\n"
	"  ------------------------------------\n"
	"      The Revenge Of Luna Lovegood    \n"
	"  ------------------------------------\n"
	"\n";

void MenuTitle::init()
{
	set_title(cstr_TitleText);
	m_options = 
	{
		{"New Game", TitleMenuOption::NewGame},
		{"Load Game", TitleMenuOption::LoadGame},
		{"Quit", TitleMenuOption::Quit}
	};
}

void MenuTitle::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		switch(get_selected().value)
		{
			case TitleMenuOption::NewGame:
				Menu::show_name_entry();
				break;
			case TitleMenuOption::LoadGame:
				Menu::show_load();
				break;
			case TitleMenuOption::Quit:
				Input::request_quit();
				break;
		}
	}
	else if (key == TK_ESCAPE)
	{
		Input::request_quit();
	}
	else
	{
		MenuList::handle_input(key);
	}
}
