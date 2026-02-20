#include "MenuDebug.h"

#include "Draw.h"
#include "Creature.h"
#include "Gingerbread.h"
#include "Player.h"
#include "Spell.h"

#include "BearLibTerminal.h"

void MenuDebug::init()
{
	set_title("Debug Cheat Menu:");
	set_options(
	{
		{"Cancel", DebugMenuOption::Cancel},
		{"Learn All Spells", DebugMenuOption::LearnAllSpells},
		{"Increase Stats", DebugMenuOption::IncreaseStats},
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
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}
