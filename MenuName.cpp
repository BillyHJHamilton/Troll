#include "MenuName.h"
#include "Colour.h"
#include "Codepoint.h"
#include "Player.h"
#include "BearLibTerminal.h"
#include <cctype>

void MenuName::draw_screen ()
{
	char const* cstr_instruction = "What is your name?";

	terminal_font("");

	terminal_print(0, 0, cstr_instruction);

	terminal_put(1, 2, Codepoint::HandRight); // hand

	terminal_color(cstr_Grey);
	for (int x = 3; x < (3 + Player::c_MaxNameLength); ++x)
	{
		terminal_put(x, 2, '_');
	}
	terminal_color(cstr_White);

	terminal_layer(1);
	terminal_print(3, 2, name.c_str());
	terminal_layer(0);
}

Input::Result MenuName::handle_input (int key)
{
	// TODO Should we store the player name in wstring to support unusual characters?

	if (key == TK_ESCAPE)
	{
		Menu::show_title();
		return Input::Result::Handled;
	}
	else if (key == TK_BACKSPACE)
	{
		if (name.size() > 0)
		{
			name.pop_back();
			return Input::Result::Handled;
		}
	}
	else if (key == TK_ENTER)
	{
		if (name.size() > 0)
		{
			Player::set_name(name);
			Menu::show_house_selection();
			return Input::Result::Handled;
		}
	}
	else if (terminal_check(TK_CHAR))
	{
		char c = terminal_state(TK_CHAR);

		if (is_valid_character(c) && name.size() < Player::c_MaxNameLength)
		{
			name.push_back(c);
		}
		return Input::Result::Handled;
	}

	return Input::Result::Skipped;
}

void MenuName::init()
{
	name.clear();
}

bool MenuName::is_valid_character (char c)
{
	// Allow a bunch of characters that are legal in filenames.
	return isalnum(c) ||
		c == ' ' || c == '.' || c == ',' || c == '!' ||
		c == '-' || c == '(' || c == ')' || c == ';' ||
		c == '@' || c == '#' || c == '$' || c == '%' ||
		c == '^' || c == '&' || c == '\'' || c == '~';
}
