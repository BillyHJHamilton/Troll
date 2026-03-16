#include "MenuTitle.h"

#include "Input.h"
#include "Game.h"

#include "BearLibTerminal.h"
       
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

char const* MenuTitle::cstr_FigureText =
"       ,m.       \n"
"      6@S63      \n"
"      s36S29     \n"
"       aS&S6l    \n"
"      T q9S~ b   \n"
"     B 7L    4b  \n"
"    d p Y     q  \n"
"   d p  1-  -lyD \n"
"  eg e ;      g' \n"
"  /'   a      a  \n"
" /     e       a \n"
"*     d        a \n"
"      R         b\n"
"      L         E\n";

char const* MenuTitle::cstr_CastleText =
"                                      \n"
"              /\\                      \n"
"         +   /..\\            A        \n"
"        / \\  l 'l    /\\     / \\       \n"
"        l l  l' l A  ll  /\\^^^^\\      \n"
"    /^\\\\l l\\ l 'l+++ ll /<>\\,,,,\\     \n"
"   /   \\ '+++l  l   \\ll l  l    l /\\  \n"
"   l O l v v l 'l ' l+++p  q  l+++l l \n"
"/''l   l v v l  l'  l   l  l  l   l \\ \n"
"1  l   l.v.v.l  l   l # l  l  lnnnl  l\n";

void MenuTitle::init()
{
	set_title(cstr_TitleText);
	m_options = 
	{
		{"New Game", TitleMenuOption::NewGame},
		{"Load Game", TitleMenuOption::LoadGame},
		{"Help", TitleMenuOption::Help},
		{"Settings", TitleMenuOption::Settings},
		{"High Scores", TitleMenuOption::HighScores},
#if _DEBUG
		{"Set Logging", TitleMenuOption::SetLogging},
#endif
		{"Quit", TitleMenuOption::Quit}
	};
}

void MenuTitle::draw_screen ()
{
	MenuList::draw_screen();

	terminal_print(80, 17, cstr_FigureText);
	terminal_print(25, 21, cstr_CastleText);
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
			case TitleMenuOption::Help:
				Menu::push();
				Menu::show_help();
				break;
			case TitleMenuOption::Settings:
				Menu::push();
				Menu::show_settings();
				break;
			case TitleMenuOption::HighScores:
				Menu::push();
				Menu::show_high_scores();
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
