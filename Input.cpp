#include "BearLibTerminal.h"

#include "Input.h"

#include "Action.h"
#include "BertieBotts.h" // test
#include "Bot.h"
#include "Confirm.h"
#include "Draw.h"
#include "Game.h"
#include "Geometry.h"
#include "Inventory.h"
#include "Map.h"
#include "Menu.h"
#include "Player.h"
#include "Spawn.h"
#include "Spell.h"
#include "Target.h"
#include "World.h"

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
CompassDirection parse_directional (int tk_code);

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

Input::Result handle_next_input ()
{
	// block until input is received
	// note: "key" may also include mouse events, etc.
	int key = terminal_read();

	//-----------------------------------------------------------
	// Skip inputs we don't care about
	
	if (key == TK_MOUSE_MOVE || key == TK_MOUSE_SCROLL)
	{
		return Result::Skipped;
	}

	//-----------------------------------------------------------
	// Global input - All Game modes

	if (key == TK_CLOSE || // X button in the corner
	   (key == TK_F4 && terminal_check(TK_ALT))) // Vulcan nerve pinch
	{
		request_quit();
		return Result::Handled;	
	}

	if (key == TK_RESIZED)
	{
		return Result::Handled; // Redraw screen after resize
	}

	//-----------------------------------------------------------
	// Menu input - Redirect to menu
	
	if (Game::get_mode() == GameMode::Menu)
	{
		return Menu::handle_input(key);
	}
	else if (Game::get_mode() == GameMode::Confirm)
	{
		return Confirm::handle_input(key);
	}

	//-----------------------------------------------------------
	// Game input - Toggling spell mode on/off

	// enter spellcasting mode when shift is pressed down
	if (key == TK_SHIFT)
	{
		s_input_mode = InputMode::Spellcasting;
		blank_spell_input();
		return Result::Handled;
	}

	// return to normal mode when shift is released
	if (key == (TK_SHIFT | TK_KEY_RELEASED))
	{
		s_input_mode = InputMode::Normal;
		if (check_spell_input_state() == SpellInputState::InProgress)
		{
			blank_spell_input();
		}
		return Result::Handled;
	}

	// And, mouse-based targeting
	if (key == TK_MOUSE_LEFT)
	{
		Draw::View const& view = Draw::get_view();
		Vec3 const mouse_pos = view.mouse_to_global_pos();
		if (view.contains_global_pos(mouse_pos))
		{
			Target::set_to(mouse_pos);
		}
		return Result::Handled;
	}

	//-----------------------------------------------------------
	// Game input - In normal mode

	if (s_input_mode == InputMode::Normal)
	{
		if (is_directional(key))
		{
			CompassDirection const dir = parse_directional(key);

			if (terminal_check(TK_CONTROL))
			{
				Player::start_automove(dir);
				return Result::StartAutomate;
			}
			else
			{
				Vec2 const vec = c_Compass[dir];
				Action::player_try_move(vec);
				return Result::Handled;
			}
		}

		if (key == TK_KP_5 || key == TK_SPACE)
		{
			if (terminal_check(TK_CONTROL))
			{
				Player::start_automove(c_CompassNoMove);
				return Result::StartAutomate;
			}
			else
			{
				Action::player_rest_step();
				return Result::Handled;
			}
		}

		// For some reason when ctrl is pressed, BearLib only issues the
		// released event for TK_TAB, not the pressed event.  :/
		// Possibly a Windows-specific issue.
		if (key == (TK_TAB | TK_KEY_RELEASED) &&
			terminal_check(TK_CONTROL))
		{
			Target::snap_to_player();
			return Result::Handled;
		}
		else if (key == TK_TAB && !terminal_check(TK_CONTROL))
		{
			Target::cycle(1, /*manually*/ true);
			return Result::Handled;
		}

		// Right-click automove
		if (key == TK_MOUSE_RIGHT)
		{
			Draw::View const& view = Draw::get_view();
			Vec3 const mouse_pos = view.mouse_to_global_pos();
			if (view.contains_global_pos(mouse_pos))
			{
				Player::start_pathfind(mouse_pos);
				return Result::StartAutomate;
			}
		}

		// Auto-collect items
		if (key == TK_C && terminal_check(TK_CONTROL))
		{
			Player::auto_collect();
			return Result::StartAutomate;
		}

		// Auto-explore (to nearest darkness)
		if (key == TK_D && terminal_check(TK_CONTROL))
		{
			Player::auto_darkness();
			return Result::StartAutomate;
		}

		//// Auto-explore (until further notice)
		// Disabled for now because I feel like the C/D version is more fun
		//if (key == TK_E && terminal_check(TK_CONTROL))
		//{
		//	Player::auto_explore();
		//	return Result::StartAutomate;
		//}

		// Go to (target location)
		if (key == TK_G && terminal_check(TK_CONTROL))
		{
			if (Target::is_valid())
			{
				Player::start_pathfind(Target::get_pos().value());
			}
			return Result::StartAutomate;
		}

		// Bean test
		//if (key == TK_B)
		//{
		//	int const flavour = BertieBotts::random_flavour();
		//	Draw::add_message(std::string("[color=") + BertieBotts::get_colour(flavour)
		//		+ std::string("] ") + BertieBotts::get_name(flavour) + std::string("[/color]"));
		//
		//	return Result::Handled;
		//}

#if _DEBUG
		if (key == TK_D)
		{
			Menu::show_debug_menu();
			return Result::Handled;
		}
#endif

		if (key == TK_H)
		{
			Menu::show_help();
			return Result::Handled;
		}

		if (key == TK_I || key == TK_ENTER)
		{
			if (terminal_check(TK_CONTROL))
			{
				int const slot = Inventory::read().find_most_recently_used();
				if (slot != c_Invalid)
				{
					Action::player_use_item(slot);
					return Result::Handled;
				}
			}

			Menu::show_inventory();
			return Result::Handled;
		}

		if (key == TK_X)
		{
			Action::player_look_at();
			return Result::Handled;
		}

		if (key == TK_ESCAPE)
		{
			Menu::show_pause_menu();
			return Result::Handled;
		}

		// Map Debug
#if _DEBUG
		if (Draw::get_view().ignore_visibility)
		{
			if (key == TK_P)
			{
				Player::handle().move(Player::pos() + Vec3{0,0,1});
				Spawn::check_spawning();
				return Result::Handled;
			}
			if (key == TK_SEMICOLON)
			{
				Player::handle().move(Player::pos() + Vec3{0,0,-1});
				Spawn::check_spawning();
				return Result::Handled;
			}
		}
#endif //_DEBUG
	}

	//-----------------------------------------------------------
	// Game input - in spellcasting mode

	if (s_input_mode == InputMode::Spellcasting)
	{
		if (key == TK_TAB)
		{
			Target::cycle(-1, /*manually*/ true);
			return Result::Handled;
		}

		if (is_letter(key))
		{
			char letter = 'A' + (key - TK_A);
			handle_input_spell_keys(letter);
			return Result::Handled;
		}

		if (is_directional(key))
		{
			Vec2 const vec = c_Compass[parse_directional(key)];
			Target::move(vec);
			return Result::Handled;
		}

		if (key == TK_SLASH)
		{
			Menu::show_spells_known();
			return Result::Handled;
		}
	}

	// unhandled
	return Result::Skipped;
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

void request_quit()
{
	// First, save the game if a save file is open.
	Game::save();

	s_quit_flag = true;
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

CompassDirection parse_directional (int tk_code)
{
	switch(tk_code)
	{
	case TK_KP_6:
	case TK_RIGHT:
		return c_CompassEast;
		break;
	case TK_KP_9:
	case TK_PAGEUP:
		return c_CompassNortheast;
		break;
	case TK_KP_8:
	case TK_UP:
		return c_CompassNorth;
		break;
	case TK_KP_7:
	case TK_HOME:
		return c_CompassNorthwest;
		break;
	case TK_KP_4:
	case TK_LEFT:
		return c_CompassWest;
		break;
	case TK_KP_1:
	case TK_END:
		return c_CompassSouthwest;
		break;
	case TK_KP_2:
	case TK_DOWN:
		return c_CompassSouth;
		break;
	case TK_KP_3:
	case TK_PAGEDOWN:
		return c_CompassSoutheast;
		break;
	default:
		assert(false);
		return c_CompassNoMove;
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
		[[fallthrough]];

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
		Action::player_try_cast_spell(s_selected_spell);
	}
}

}