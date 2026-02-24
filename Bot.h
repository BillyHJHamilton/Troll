#pragma once

#include "Types.h"

#include "Creature.h"

// AI functions

namespace Bot
{
	enum State : byte
	{
		Rest,
		Blunder,
		Chase,
		Fight,
	};

	// Data for an AI creature.
	struct Brain
	{
		std::vector<Vec3> move_stack;
		Vec3 target_pos {};
		Creature::Handle target = c_Invalid;
		int awareness = 0;
		int patience = 0;
		int pathfind_ready = 0;
		Bot::State state = Rest;

		void serialize(ISerializer& s);
	};

	void clear();
	void serialize(ISerializer& s);

	void init_brain(Creature::Handle creature);

	void do_turn (Creature::Handle creature_index);
}
