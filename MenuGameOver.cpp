#include "MenuGameOver.h"

#include "Cloud.h"
#include "Damage.h"
#include "Debug.h"
#include "Game.h"
#include "Grammar.h"
#include "Player.h"
#include "Status.h"

#include "BearLibTerminal.h"
#include <format>

void MenuGameOver::refresh()
{
	m_text = "Game Over.";

	Damage::Cause defeated_by = Player::get_defeated_by();

	if (defeated_by.type == Damage::Cause::Creature)
	{
		Creature::Type const creature_type = (Creature::Type)defeated_by.index;

		if (creature_type == Creature::Player)
		{
			m_text = "Game Over.\n\n"
			"You were defeated by ... yourself.";
		}
		else if (creature_type != Creature::None)
		{
			m_text = std::format(
				"Game Over.\n\n"
				"You were defeated by {}.",
				Grammar::format_name_by_type(creature_type,
					{ .long_name = true, .mode = Grammar::NameParam::Indefinite }));
		}
	}
	else if (defeated_by.type == Damage::Cause::Status)
	{
		Status::Index const status = (Status::Index)defeated_by.index;
		char const* predicate = nullptr;
		switch (status)
		{
			case Status::Dancing:
				predicate = "careless dancing";
				break;
			case Status::Burning:
				predicate = "catching on fire";
				break;
			default:
				DebugBreak("Missing string for defeated by status.");
				predicate = "an unexpected situation";
				break;
		}

		m_text = std::format(
			"Game Over.\n\n"
			"You were defeated by {}.",
			predicate);
	}
	else if (defeated_by.type == Damage::Cause::Cloud)
	{
		Cloud::Type const cloud = (Cloud::Type)defeated_by.index;
		char const* predicate = nullptr;
		switch (cloud)
		{
			case Cloud::Slime:
				predicate = "acidic slime";
				break;
			default:
				DebugBreak("Missing string for defeated by cloud.");
				predicate = "a cloud of errors";
				break;
		}

		m_text = std::format(
			"Game Over.\n\n"
			"You were defeated by {}.",
			predicate);
	}
}

void MenuGameOver::draw_screen ()
{
	terminal_font("");
	terminal_print(0, 0, m_text.c_str());
}

void MenuGameOver::handle_input (int key)
{
	switch(key)
	{
		case TK_ENTER:
		case TK_ESCAPE:
			Game::reset();
			break;
	}
}
