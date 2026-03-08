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
		int last_taunt = c_Invalid;
		Bot::State state = Rest;

		void serialize(ISerializer& s);
	};

	void init();
	void clear();
	void serialize(ISerializer& s);

	void init_brain(Creature::Handle creature);

	// Player pathfinding bot
	bool try_player_pathfind(Vec3 goal);
	bool try_player_collect();
	bool try_player_explore();
	bool has_player_path();
	Vec2 pop_player_path(); // returns {0,0} on failure
	void clear_player_path();

	// Enemy creature bot
	void do_turn(Creature::Handle creature_index);

	// Let a creature know something tried to hit it.
	void notice_attack(Creature::Handle creature, Vec3 attack_origin);
}
