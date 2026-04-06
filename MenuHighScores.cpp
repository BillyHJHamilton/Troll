#include "MenuHighScores.h"

#include "Colour.h"
#include "Game.h"
#include "Player.h"
#include "Score.h"

#include "BearLibTerminal.h"
#include <format>

void MenuHighScores::show_game_over()
{
	// Submit the player's score!
	Score::Entry const& new_entry = Score::make_player_score();

	m_mode = Mode::GameOver;

	if (new_entry.ending == Score::Ending::Defeated)
	{
		m_text = std::format("Game Over.\n\n"
			"You were defeated by {}.\n"
			"Total score: {} points",
			new_entry.defeated_by, new_entry.points);
	}
	else if (new_entry.ending == Score::Ending::Won)
	{
		m_text = std::format("Game Over.\n\n"
			"You won the game!\n"
			"Total score: {} points",
			new_entry.points);
	}
}

void MenuHighScores::show_scores()
{
	m_mode = Mode::DisplayOnly;
	m_text = "High scores:";
}

void MenuHighScores::draw_screen ()
{
	terminal_font("");
	dimensions_t dim = terminal_print(0, 0, m_text.c_str());

	int const highlight = Score::active_row();

	if (Score::num_scores() == 0)
	{
		terminal_print(0,dim.height + 1,"(No scores recorded.)");
	}

	for (int row = 0; row < Score::num_scores(); ++row)
	{
		int const y = dim.height + 1 + row;
		Score::Entry const& entry = Score::read_entry(row);

		if (row == highlight)
		{
			terminal_color(cstr_Yellow);
		}

		terminal_printf_ext(0,y,4,1,TK_ALIGN_RIGHT, "%d)", row+1);
		terminal_printf_ext(5,y,Player::c_MaxNameLength,1,TK_ALIGN_LEFT, "%s", entry.name.c_str());
		terminal_printf_ext(Player::c_MaxNameLength + 5,y,12,1,TK_ALIGN_RIGHT, "%d Pts", entry.points);

		if (entry.ending == Score::Ending::Defeated)
		{
			terminal_printf_ext(Player::c_MaxNameLength + 20,y,40,1,TK_ALIGN_LEFT, "Defeated by %s.", entry.defeated_by.c_str());
		}
		else
		{
			terminal_print_ext(Player::c_MaxNameLength + 20,y,40,1,TK_ALIGN_LEFT, "Won the game!");
		}

		if (row == highlight)
		{
			terminal_color(cstr_White);
		}
	}
}

Input::Result MenuHighScores::handle_input (int key)
{
	switch(key)
	{
		case TK_ENTER:
		case TK_ESCAPE:
			if (m_mode == Mode::GameOver)
			{
				Game::reset();
			}
			else
			{
				Menu::back();
			}
			return Input::Result::Handled;
	}

	return Input::Result::Skipped;
}
