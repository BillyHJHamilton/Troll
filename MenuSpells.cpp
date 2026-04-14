#include "MenuSpells.h"

#include "Action.h"
#include "Creature.h"
#include "Colour.h"
#include "Draw.h"
#include "Game.h"
#include "Player.h"
#include "Spell.h"
#include "Target.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <iomanip>
#include <format>

void MenuSpells::draw_screen ()
{
	MenuList::draw_screen();
	draw_selected_spell();
}

Input::Result MenuSpells::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		if (m_mode == Mode::StartingSpells)
		{
			select_starting_spell();
			return Input::Result::Handled;
		}
		else if (m_mode == Mode::KnownSpells)
		{
			Menu::close();
			Spell::Index spell = (Spell::Index)get_selected().value;
			Draw::add_message(std::format("To cast {}, hold shift and type {}.",
				Spell::get_name(spell), Spell::get_abbrev(spell)));
			//player_try_cast_spell((Spell::Index)get_selected().value);

			return Input::Result::Handled;
		}
	}
	else if (key == TK_ESCAPE)
	{
		if (m_mode == Mode::StartingSpells)
		{
			Menu::show_house_selection();
		}
		else
		{
			Menu::back();
		}
		return Input::Result::Handled;
	}

	return MenuList::handle_input(key);
}

void MenuSpells::show_known_spells ()
{
	clear_list();
	m_mode = Mode::KnownSpells;

	set_title("Known spells:");
	
	Spell::TempList spells_known = Player::handle().spells_known();
	reserve(Util::Size(spells_known));

	for (Spell::Index spell_index : spells_known)
	{
		std::string label = Spell::get_abbrev(spell_index) + " " + Spell::get_name(spell_index);
		int const value = (int)spell_index;
		add_option(label, value);
	}
}

void MenuSpells::show_starting_spells ()
{
	clear_list();
	m_mode = Mode::StartingSpells;

	set_title("Choose three spells to start with:");
	reserve(10);

	int constexpr c_MaxDifficulty = 25;

	for (Spell::Index spell_index = (Spell::Index)0;
		Spell::is_valid_index(spell_index);
		spell_index = (Spell::Index)(spell_index + 1))
	{
		if (Spell::get_difficulty(spell_index) <= c_MaxDifficulty)
		{
			std::string label = Spell::get_abbrev(spell_index) + " " + Spell::get_name(spell_index);
			int const value = (int)spell_index;
			add_option(label, value);
		}
	}

	m_num_selected = 0;
}

void MenuSpells::draw_selected_spell ()
{
	Spell::Index s = (Spell::Index)get_selected().value;
	if (!Spell::is_valid_index(s))
	{
		return;
	}

	float const base_success = 100.0f - Spell::get_miscast_rate(s, Player::handle().skill_magic());
	int const damage = Spell::get_damage(s, Player::handle());
	int const range = Spell::get_range(s);
	Target::Type target_type = Spell::get_target_type(s);

	std::stringstream ss;
	ss << Spell::get_name(s) << "\n\n";
	ss << std::setw(13) << std::left << "Difficulty:"
		<< std::setw(3) << std::left << Spell::get_difficulty(s)
		<< " (" << std::fixed << std::setprecision(0) << base_success << "% successful)\n";

	if (target_type == Target::Beam || target_type == Target::Melee)
	{
		ss << std::setw(13) << std::left << "Accuracy:" << Spell::get_accuracy(s) << "\n";
	}

	if (range > 0)
	{
		ss << std::setw(13) << std::left << "Range:" << range << "\n";
	}

	if (damage > 0)
	{
		ss << std::setw(13) << std::left << "Damage:" << damage << "\n";
	}

	dimensions_t dim = terminal_print(50, 2, ss.str().c_str());
	terminal_print_ext(50, 2 + dim.height, 60, 20, 1, Spell::get_description(s));
}

void MenuSpells::select_starting_spell ()
{
	if (m_options[m_cursor].colour == nullptr)
	{
		++m_num_selected;
		m_options[m_cursor].colour = cstr_LightYellow;
	}
	else
	{
		--m_num_selected;
		m_options[m_cursor].colour = nullptr;
	}

	if (m_num_selected == 3)
	{
		// That's it!  Start the game!
		for (int i = 0; i < Util::Size(m_options); ++i)
		{
			if (m_options[i].colour != nullptr)
			{
				Spell::Index spell = (Spell::Index)m_options[i].value;
				Player::handle().learn_spell(spell);
			}
		}

		Game::setup();
		Menu::show_prologue();
	}
}
