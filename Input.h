#pragma once
#include "Geometry.h"

namespace Input
{
	void clear();

	void handle_next_input();
	void dispatch_automove();
	std::string get_spell_preview_string();

	bool is_quitting();
}