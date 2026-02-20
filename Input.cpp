#include "BearLibTerminal.h"

#include "Input.h"

#include "Action.h"
#include "BertieBotts.h" // test
#include "Bot.h"
#include "Draw.h"
#include "Game.h"
#include "Geometry.h"
#include "Inventory.h"
#include "Map.h"
#include "Menu.h"
#include "Player.h"
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

void handle_next_input ()
{
	// block until input is received
	// note: "key" may also include mouse events, etc.
	int key = terminal_read();

	//-----------------------------------------------------------
	// Global input - All Game modes

	if (key == TK_CLOSE || // X button in the corner
	   (key == TK_F4 && terminal_check(TK_ALT))) // Vulcan nerve pinch
	{
		request_quit();
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
		Player::stop_automove();
		s_input_mode = InputMode::Spellcasting;
		blank_spell_input();
		return;
	}

	// return to normal mode when shift is released
	if (key == (TK_SHIFT | TK_KEY_RELEASED))
	{
		Player::stop_automove();
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
			CompassDirection const dir = parse_directional(key);

			if (terminal_check(TK_CONTROL))
			{
				Player::start_automove(dir);
			}
			else
			{
				Player::stop_automove();
				Vec2 const vec = c_Compass[dir];
				player_try_move(vec);
				return;
			}
		}

		if (key == TK_KP_5 || key == TK_SPACE)
		{
			if (terminal_check(TK_CONTROL))
			{
				Player::start_automove(c_CompassNoMove);
			}
			else
			{
				Player::stop_automove();
				player_rest_step();
			}
			return;
		}

		if (key == TK_TAB)
		{
			Player::stop_automove();
			Target::cycle();
			return;
		}

		// Bean test
		//if (key == TK_B)
		//{
		//	Player::stop_automove();
		//
		//	int const flavour = BertieBotts::random_flavour();
		//	Draw::add_message(std::string("[color=") + BertieBotts::get_colour(flavour)
		//		+ std::string("] ") + BertieBotts::get_name(flavour) + std::string("[/color]"));
		//
		//	return;
		//}

#if _DEBUG
		if (key == TK_D)
		{
			Player::stop_automove();
			Menu::show_debug_menu();
			return;
		}
#endif

		if (key == TK_H)
		{
			Player::stop_automove();
			Menu::show_help();
			return;
		}

		if (key == TK_I)
		{
			Player::stop_automove();
			Menu::show_inventory();
			return;
		}

		if (key == TK_ESCAPE)
		{
			Player::stop_automove();
			Menu::show_pause_menu();
			return;
		}

		// Map Debug
		//if (key == TK_PERIOD)
		//{
		//	Player::handle().move(Player::pos() + Vec3{0,0,-1});
		//	return;
		//}
		//if (key == TK_COMMA)
		//{
		//	Player::handle().move(Player::pos() + Vec3{0,0,1});
		//	return;
		//}
		//if (key == TK_X)
		//{
		//	Draw::toggle_los_cheat();
		//	//int const map_index = World::read().find_map(Player::pos());
		//	//if (map_index != c_Invalid)
		//	//{
		//	//	World::edit().edit_map(map_index).set_all_explored();
		//	//}
		//	return;
		//}

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
			Vec2 const vec = c_Compass[parse_directional(key)];
			Target::move(vec);
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

void dispatch_automove()
{
	CompassDirection const dir = Player::get_automove();
	if (dir == c_CompassNoMove)
	{
		player_rest_step();

		if (!Player::handle().is_hurt())
		{
			Player::stop_automove();
		}
	}
	else
	{
		// Automove behaviour, inspired by run system in Linley's Dungeon Crawl.
		CompassDirection const clockwise = get_clockwise(dir);
		CompassDirection const counterclockwise = get_counterclockwise(dir);
		Vec3 const p0 = Player::pos();
		Vec3 const p1 = p0 + c_Compass[clockwise].xy0();
		Vec3 const p2 = p0 + c_Compass[counterclockwise].xy0();
		Terrain::Type t0 = World::read().get_terrain(p0);
		Terrain::Type t1 = World::read().get_terrain(p1);
		Terrain::Type t2 = World::read().get_terrain(p2);

		bool const moved = player_try_move(c_Compass[dir]);

		if (!moved)
		{
			Player::stop_automove();
		}
		else
		{
			Vec3 const new_p0 = Player::pos();
			Vec3 const new_p1 = new_p0 + c_Compass[clockwise].xy0();
			Vec3 const new_p2 = new_p0 + c_Compass[counterclockwise].xy0();
			Terrain::Type new_t0 = World::read().get_terrain(new_p0);
			Terrain::Type new_t1 = World::read().get_terrain(new_p1);
			Terrain::Type new_t2 = World::read().get_terrain(new_p2);

			if (t0 != new_t0 || t1 != new_t1 || t2 != new_t2)
			{
				Player::stop_automove();
			}
		}
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
		player_try_cast_spell(s_selected_spell);
	}
}

}