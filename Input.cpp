#include "BearLibTerminal.h"

#include "Input.h"

#include "Action.h"
#include "Bot.h"
#include "Draw.h"
#include "Game.h"
#include "Geometry.h"
#include "Menu.h"
#include "Player.h"
#include "Spell.h"
#include "Target.h"

#include <cassert>
#include <iostream>

namespace Input
{

enum class InputMode
{
	Normal,
	Spellcasting
};

enum class SpellInputState
{
	Empty,
	InProgress,
	Invalid,
	Success
};

static InputMode s_input_mode;
static char s_spellcode [3];
static Spell::Index s_selected_spell;

bool s_quit_flag = false;

//-------------------------------------------------------------------------------------------------
// Helper function declarations

bool is_letter (int tk_code);
bool is_keyboard_key (int tk_code);
bool is_directional (int tk_code);
Vec2 parse_directional (int tk_code);

void handle_input_close ();

void blank_spell_input ();
SpellInputState check_spell_input_state ();
void handle_input_spell_keys (char letter);
void handle_spellcode_complete ();

//-------------------------------------------------------------------------------------------------
// Interface function implementations

void clear ()
{
	blank_spell_input();
	s_input_mode = InputMode::Normal;
	s_selected_spell = Spell::None;
}

void handle_next_input ()
{
	// block until input is received
	// note: "key" may also include mouse events, etc.
	int key = terminal_read();

	//int shift = terminal_check(TK_SHIFT);

	//-----------------------------------------------------------
	// Global input - All Game modes

	if (key == TK_CLOSE) // X button in the corner
	{
		handle_input_close();
		return;	
	}

	//-----------------------------------------------------------
	// Menu input - Redirect to menu
	
	if (Game::get_mode() == GameMode::Menu)
	{
		Menu::handle_input(key);
		return;
	}

	//-----------------------------------------------------------
	// Game input - Toggling spell mode on/off

	// enter spellcasting mode when shift is pressed down
	if (key == TK_SHIFT)
	{
		s_input_mode = InputMode::Spellcasting;
		blank_spell_input();
		return;
	}

	// return to normal mode when shift is released
	if (key == (TK_SHIFT | TK_KEY_RELEASED))
	{
		s_input_mode = InputMode::Normal;
		if (check_spell_input_state() == SpellInputState::InProgress)
		{
			blank_spell_input();
		}
		return;
	}

	//-----------------------------------------------------------
	// Game input - In normal mode

	if (s_input_mode == InputMode::Normal)
	{
		if (is_directional(key))
		{
			Vec2 dir = parse_directional(key);
			player_try_move(dir);
			return;
		}

		if (key == TK_TAB)
		{
			Target::cycle();
			return;
		}

		//if (key == TK_SPACE)
		//{
		//	Bot::do_all_bot_turns();
		//	return;
		//}

		if (key == TK_H)
		{
			Menu::show_help();
			return;
		}

		// unhandled
		return;
	}

	//-----------------------------------------------------------
	// Game input - in spellcasting mode

	if (s_input_mode == InputMode::Spellcasting)
	{
		if (is_letter(key))
		{
			char letter = 'A' + (key - TK_A);
			handle_input_spell_keys(letter);
			return;
		}

		if (is_directional(key))
		{
			Vec2 dir = parse_directional(key);
			Target::move(dir);
			return;
		}

		if (key == TK_SLASH)
		{
			Menu::show_spells_known();
			return;
		}

		// unhandled
		return;
	}
}

std::string get_spell_preview_string ()
{
	SpellInputState state = check_spell_input_state();

	switch(state)
	{
	case SpellInputState::Success:
		return Spell::get_abbrev(s_selected_spell)
			+ " - " + Spell::get_name(s_selected_spell);

	case SpellInputState::Invalid:
		return std::string(s_spellcode) + " - Invalid";

	case SpellInputState::InProgress:
		return std::string(s_spellcode);

	case SpellInputState::Empty:
		if (s_input_mode == InputMode::Normal)
		{
			return "";
		}
		else
		{
			return "__";
		}

	default:
		assert(false);
		return "";
	}
}

bool is_quitting()
{
	return s_quit_flag;
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

bool is_letter (int tk_code)
{
	return tk_code >= TK_A && tk_code <= TK_Z;
}

bool is_keyboard_key (int tk_code) // still used?
{
	return (tk_code >= TK_A && tk_code <= TK_ALT);
}

bool is_directional (int tk_code)
{
	return (tk_code >= TK_RIGHT && tk_code <= TK_UP)
		|| (tk_code >= TK_KP_1 && tk_code <= TK_KP_9 && tk_code != TK_KP_5)
		|| tk_code == TK_HOME || tk_code == TK_END
		|| tk_code == TK_PAGEUP || tk_code == TK_PAGEDOWN;
}

Vec2 parse_directional (int tk_code)
{
	switch(tk_code)
	{
	case TK_KP_6:
	case TK_RIGHT:
		return {1,0};
		break;
	case TK_KP_9:
	case TK_PAGEUP:
		return {1,-1};
		break;
	case TK_KP_8:
	case TK_UP:
		return {0,-1};
		break;
	case TK_KP_7:
	case TK_HOME:
		return {-1,-1};
		break;
	case TK_KP_4:
	case TK_LEFT:
		return {-1,0};
		break;
	case TK_KP_1:
	case TK_END:
		return {-1,1};
		break;
	case TK_KP_2:
	case TK_DOWN:
		return {0,1};
		break;
	case TK_KP_3:
	case TK_PAGEDOWN:
		return {1,1};
		break;
	default:
		assert(false);
		return {0,0};
	}
}

void blank_spell_input ()
{
	s_spellcode[0] = 0;
	s_spellcode[1] = 0;
	s_spellcode[2] = 0;
}

SpellInputState check_spell_input_state ()
{
	if (s_spellcode[0] == 0)
	{
		return SpellInputState::Empty;
	}
	else if (s_spellcode[1] == 0)
	{
		return SpellInputState::InProgress;
	}
	else if (s_selected_spell == Spell::None)
	{
		return SpellInputState::Invalid;
	}
	else
	{
		return SpellInputState::Success;
	}
}

void handle_input_spell_keys (char letter)
{
	SpellInputState state = check_spell_input_state();

	switch (state)
	{
	case SpellInputState::Invalid:
	case SpellInputState::Success:
		blank_spell_input();
		s_selected_spell = Spell::None;
		// and fall through to empty

	case SpellInputState::Empty:
		s_spellcode[0] = letter;
		break;

	case SpellInputState::InProgress:
		s_spellcode[1] = letter;
		handle_spellcode_complete();
	}
}

void handle_spellcode_complete ()
{
	// check if it's a real spell
	Spell::Index spell_index = Spell::get_index_by_abbrev(s_spellcode);

	if (spell_index == Spell::None)
	{
		s_selected_spell = Spell::None;
	}
	else
	{
		s_selected_spell = spell_index;
		player_try_cast_spell(s_selected_spell);
	}
}

void handle_input_close()
{
	s_quit_flag = true;
}

}