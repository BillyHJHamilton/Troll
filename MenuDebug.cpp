#include "MenuDebug.h"

#if _DEBUG

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
				Menu::close();
				Draw::add_message("Your knowledge returns from future past.");
				break;
			case DebugMenuOption::IncreaseStats:
				Gingerbread::edit_player_stats().max_hp += 100;
				Gingerbread::edit_player_stats().skill_magic += 100;
				Player::handle().heal_hp(100);
				Menu::close();
				Draw::add_message("You feel remarkably fit.");
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

void MenuDebugLogCategories::refresh()
{
	set_title("Log Categories:");

	m_options.resize(Debug::Category::Count + 2);
	for (int i = 0; i < Debug::Category::Count; ++i)
	{
		bool const enabled = Debug::enabled((Debug::Category)i);
		m_options[i].label = std::format("[[{}]] {}",
			enabled ? "ON" : "  ",
			Debug::category_name((Debug::Category)i));
		m_options[i].value = i;
	}
	m_options[Debug::Category::Count] = {"Enable All", c_EnableAll};
	m_options[Debug::Category::Count + 1] = {"Disable All", c_DisableAll};
}

void MenuDebugLogCategories::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		int const value = get_selected().value;
		if (value == c_EnableAll)
		{
			Debug::set_all_enabled(true);
			refresh();
		}
		else if (value == c_DisableAll)
		{
			Debug::set_all_enabled(false);
			refresh();
		}
		else
		{
			Debug::Category category = (Debug::Category)value;
			Debug::set_enabled(category, !Debug::enabled(category));
			refresh();
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}

#endif // _DEBUG
