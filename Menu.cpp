#include "Menu.h"

#include "BearLibTerminal.h"

#include "Draw.h"
#include "Global.h"

namespace Menu
{

//-------------------------------------------------------------------------------------------------
// Data

Type s_current_menu_type = Type::None;

// For document menus.
const char* s_document_content = nullptr;

const char* const c_doc_help =
	"Move - Arrow keys or numpad \n"
	"Diagonals - Numpad or home/end/pgup/pgdn \n"
	"Cycle target - Tab \n"
	"Cast spell - Hold shift, type 2-letter abbreviation \n"
	"Target position - Hold shift, use movement keys \n"
	"List spells - (todo) \n\n"
	"(press enter)";

//-------------------------------------------------------------------------------------------------
// Helper function declarations

void draw_document();

//-------------------------------------------------------------------------------------------------
// Public function implementations

void show_help()
{
	s_current_menu_type = Document;
	s_document_content = c_doc_help;
	g_game_mode = GameMode::Menu;
}

void close()
{
	s_current_menu_type = None;
	g_game_mode = GameMode::Normal;
}

void update_screen()
{
	switch (s_current_menu_type)
	{
		case Document:
			draw_document();
			break;
	}
}

void handle_input(int key)
{
	if (key == TK_ENTER)
	{
		Menu::close();
	}
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

void draw_document()
{
	terminal_font("");
	terminal_print(0, 0, s_document_content);
}

}
