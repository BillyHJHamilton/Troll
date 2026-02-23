#include "MenuTitle.h"

#include "Input.h"
#include "Game.h"

#include "BearLibTerminal.h"

// d888888b d8888b.  .d88b.  db      db      
// `~~88~~` 88  `8D .8P  Y8. 88      88      
//    88    88oobY` 88    88 88      88      
//    88    88`8b   88    88 88      88      
//    88    88 `88. `8b  d8` 88booo. 88booo. 
//    YP    88   YD  `Y88P'  Y88888P Y88888P 

//char const* MenuTitle::cstr_TitleText =
//	"\n"
//	"  ------------------------------------\n\n"
//	"  TTTTTTT RRRR    OOO    L      L     \n"
//	"     T    R   R  O   O   L      L     \n"
//	"     T    R  R  O     O  L      L     \n"
//	"     T    RRR   O     O  L      L     \n"
//	"     T    R  R   O   O   L      L     \n"
//	"     T    R   R   OOO    LLLLLL LLLLLL\n\n"
//	"  ------------------------------------\n"
//	"      The Revenge Of Luna Lovegood    \n"
//	"  ------------------------------------\n"
//	"\n";
       
char const* MenuTitle::cstr_TitleText = "\n"
"  ==============================================\n"
"                                     		     \n"
"   aa@@@@aa a@@@b    ad@@ba  qa@e    qa&p       \n"
"    ~~@@~~  @@~~@@  d@P~~Y@b  @@*     @@        \n"
"      @@    @@  @a  @@   *@@  @@      @@        \n"
"      @@    @@@@b   @@    @@  @@      @@        \n"
"      @@    @@*q@b  *@b  d@P  @@aeba  @@&adb    \n"
"      @@    @@  ~@b  aq@@@P  d@@@@@@a @$@@@@a   \n"
"  											     \n"
"  ==============================================\n"
"       THE   REVENGE   OF   LUNA   LOVEGOOD     \n"
"  ==============================================\n\n";

void MenuTitle::init()
{
	set_title(cstr_TitleText);
	m_options = 
	{
		{"New Game", TitleMenuOption::NewGame},
		{"Load Game", TitleMenuOption::LoadGame},
		{"Settings", TitleMenuOption::Settings},
#if _DEBUG
		{"Set Logging", TitleMenuOption::SetLogging},
#endif
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
				Menu::push();
				Menu::show_load();
				break;
			case TitleMenuOption::Settings:
				Menu::push();
				Menu::show_settings();
				break;
#if _DEBUG
			case TitleMenuOption::SetLogging:
				Menu::push();
				Menu::show_debug_log_categories();
				break;
#endif
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
