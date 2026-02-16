#include "MenuSpells.h"

#include "Action.h"
#include "Creature.h"
#include "Draw.h"
#include "Player.h"
#include "Spell.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <iomanip>
#include <format>

void MenuSpells::draw_screen ()
{
	MenuList::draw_screen();
	draw_selected_spell();
}

void MenuSpells::handle_input (int key)
{
	if (key == TK_ENTER && m_mode == Mode::StartingSpells)
	{
		select_starting_spell();
	}
	else if (key == TK_ENTER && m_mode == Mode::KnownSpells)
	{
		Menu::close();
		Spell::Index spell = (Spell::Index)get_selected().value;
		Draw::add_message(std::format("To cast {}, hold shift and type {}.",
			Spell::get_name(spell), Spell::get_abbrev(spell)));
		//player_try_cast_spell((Spell::Index)get_selected().value);
	}
	else
	{
		MenuList::handle_input(key);
	}
}

void MenuSpells::show_known_spells ()
{
	clear_list();
	m_mode = Mode::KnownSpells;

	set_title("Known spells:");
	
	std::vector<Spell::Index> spells_known = Player::handle().spells_known();
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
	Spell::TargetType target_type = Spell::get_target_type(s);

	std::stringstream ss;
	ss << Spell::get_name(s) << "\n\n";
	ss << std::setw(13) << std::left << "Difficulty:"
		<< std::setw(3) << std::left << Spell::get_difficulty(s)
		<< " (" << std::fixed << std::setprecision(0) << base_success << "%)\n";

	if (target_type == Spell::TargetType::Creature)
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
	Spell::Index spell = (Spell::Index)get_selected().value;
	Player::handle().learn_spell(spell);

	if (Player::handle().spells_known().size() >= 3)
	{
		Menu::close();
	}
	else
	{
		remove_selected();
	}
}
