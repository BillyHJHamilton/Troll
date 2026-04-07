#pragma once
#include "Types.h"

#include "Geometry.h"

#include <string>

namespace Input
{
	enum class Result : byte
	{
		Skipped = 0,
		Handled = 1,
		StartAutomate = 2,
	};

	void clear();

	// Returns true if some meaningful input was processed.
	Input::Result handle_next_input();

	std::string get_spell_preview_string();

	void request_quit();
	bool is_quitting();
}
