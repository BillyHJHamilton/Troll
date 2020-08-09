#include "BearLibTerminal.h"

#include "Input.h"

#include "Action.h"
#include "Draw.h"
#include "Geometry.h"
#include "Global.h"
#include "Player.h"
#include "Spell.h"
#include "Target.h"

#include <iostream>

static char spellcode [3];

bool is_letter(int tk_code)
{
	return tk_code >= TK_A && tk_code <= TK_Z;
}

bool is_keyboad_key(int tk_code)
{
	return (tk_code >= TK_A && tk_code <= TK_ALT);
}

void handle_next_input()
{
	// block until input is received
	// note: "key" may also include mouse events, etc.
	int key = terminal_read();
	int shift = terminal_check(TK_SHIFT);

	// check for the lowercase letters (spell keys)
	if (is_letter(key) && !shift)
	{
		char letter = 'a' + (key - TK_A);
		handle_input_lowercase(letter);
		return;
	}
	else if (is_keyboad_key(key)) // other keyboard
	{
		blank_lowercase_input();
		set_spell_preview_string("__");
	}

	// other commands
	switch(key)
	{
	case TK_KP_6:
	case TK_RIGHT:
		handle_input_walk({1,0});
		break;
	case TK_KP_9:
		handle_input_walk({1,-1});
		break;
	case TK_KP_8:
	case TK_UP:
		handle_input_walk({0,-1});
		break;
	case TK_KP_7:
		handle_input_walk({-1,-1});
		break;
	case TK_KP_4:
	case TK_LEFT:
		handle_input_walk({-1,0});
		break;
	case TK_KP_1:
		handle_input_walk({-1,1});
		break;
	case TK_KP_2:
	case TK_DOWN:
		handle_input_walk({0,1});
		break;
	case TK_KP_3:
		handle_input_walk({1,1});
		break;

	case TK_TAB:
		cycle_target();
		break;

	case TK_CLOSE: // X button in the corner
		handle_input_close();
		break;	
	}
}

void clear_input ()
{
	blank_lowercase_input();
}

void blank_lowercase_input ()
{
	spellcode[0] = 0;
	spellcode[1] = 0;
	spellcode[2] = 0;
}

void handle_input_lowercase (char letter)
{
	if (spellcode[0] == 0)
	{
		spellcode[0] = letter;
		std::string preview;
		preview += letter;
		preview += "_";
		set_spell_preview_string(preview);
	}
	else
	{
		spellcode[1] = letter;
		player_try_cast_spell(spellcode);
		blank_lowercase_input();
	}
}

void handle_input_walk(Vec2 walk_vec)
{
	Player::try_move(walk_vec);
}

void handle_input_close()
{
	g_quit_flag = true;
}
