#pragma once

#include "Types.h"

#include "Creature.h"

// AI functions

namespace Bot
{
	void do_all_bot_turns ();
	void do_turn (Creature::Handle creature_index);
}
