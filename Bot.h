#pragma once

#include "Types.h"

#include "Creature.h"

// AI functions

namespace Bot
{
	// Data for an AI creature.
	struct Brain
	{
		int awareness = 0;
	};

	void init_brain(Creature::Handle creature);

	void do_turn (Creature::Handle creature_index);
}
