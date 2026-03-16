#pragma once

#include "Types.h"

#include <string>

namespace Score
{
	enum class Ending : int
	{
		Defeated = 0,
		Won = 1
	};

	struct Entry
	{
		std::string name;
		std::string defeated_by;
		// Could add: map name.
		Ending ending = Ending::Defeated;
		int points = 0;
	};

	void init();	// Loads high scores from the file.
	void clear();	// Clears active row (doesn't delete the table).

	// Computes player's score, adds it to the table, saves it, and returns the score.
	Entry const& make_player_score();

	int num_scores();
	int active_row();
	Entry const& read_entry(int rank);
}
