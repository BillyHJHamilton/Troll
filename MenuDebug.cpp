#include "MenuDebug.h"

#if _DEBUG

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Creature.h"
#include "Gingerbread.h"
#include "Player.h"
#include "Spell.h"

#include <format>
#include "BearLibTerminal.h"

//-------------------------------------------------------------------------------------------------
// Main debug menu

void MenuDebug::init()
{
	set_title("Debug Cheat Menu:");
	set_options(
	{
		{"Cancel", DebugMenuOption::Cancel},
		{"Learn All Spells", DebugMenuOption::LearnAllSpells},
		{"Increase Stats", DebugMenuOption::IncreaseStats},
		{"Lower Magic Skill", DebugMenuOption::LowerMagicSkill},
		{"Toggle Reveal Map", DebugMenuOption::ToggleRevealMap},
		{"Defeat All Enemies", DebugMenuOption::DefeatAllEnemies},
		{"Set Log Categories", DebugMenuOption::SetLogCategories},
	});
}

void MenuDebug::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		switch(get_selected().value)
		{
			case DebugMenuOption::Cancel:
				Menu::close();
				break;
			case DebugMenuOption::LearnAllSpells:
				for (int i = 0; i < Spell::Count; ++i)
				{
					Player::handle().learn_spell((Spell::Index)i);
				}
				//Menu::close();
				Draw::add_message("Your knowledge returns from future past.");
				break;
			case DebugMenuOption::IncreaseStats:
				Gingerbread::edit_player_stats().max_hp += 100;
				Gingerbread::edit_player_stats().skill_magic += 100;
				Player::handle().heal_hp(100);
				//Menu::close();
				Draw::add_message("You feel remarkably fit.");
				break;
			case DebugMenuOption::LowerMagicSkill:
				Gingerbread::edit_player_stats().skill_magic = 10;
				Draw::add_message("You feel unskillful.");
				break;
			case DebugMenuOption::ToggleRevealMap:
				Draw::toggle_los_cheat();
				//Menu::close();
				break;
			case DebugMenuOption::DefeatAllEnemies:
				for (Creature::HandleItr itr(1); itr; ++itr)
				{
					itr->take_damage({1000, Damage::Basic, {}});
				}
				//Menu::close();
				break;
			case DebugMenuOption::SetLogCategories:
				Menu::push();
				Menu::show_debug_log_categories();
				break;
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}

//-------------------------------------------------------------------------------------------------
// Debug log category menu

void MenuDebugLogCategories::init()
{
	set_title("Log Categories:");
	reserve(Debug::Category::Count + 2);

	add_option("Back", c_Invalid);

	for (int i = 0; i < Debug::Category::Count; ++i)
	{
		Debug::Category category = (Debug::Category)(i);
		add_option(Debug::category_name(category), i);
	}

	add_option("Enable All", c_EnableAll);
	add_option("Disable All", c_DisableAll);
}

void MenuDebugLogCategories::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		int const value = get_selected().value;
		if (value == c_Invalid)
		{
			Menu::back();
		}
		else if (value == c_EnableAll)
		{
			Debug::set_all_enabled(true);
		}
		else if (value == c_DisableAll)
		{
			Debug::set_all_enabled(false);
		}
		else
		{
			MenuList::handle_input(key);
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}

bool MenuDebugLogCategories::is_toggle (int option)
{
	return option > (int)Debug::Category::None &&
		option < (int)Debug::Category::Count;
}

bool MenuDebugLogCategories::get_toggle_value (int option)
{
	return Debug::enabled((Debug::Category)option);
}

void MenuDebugLogCategories::on_toggle(int option, bool new_value)
{
	Debug::set_enabled((Debug::Category)option, new_value);
}

#endif // _DEBUG
