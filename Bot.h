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
		Regroup,
		Investigate,
		Chase,
		Fight,
		Shopkeep,
	};

	// Data for an AI creature.
	struct Brain
	{
		Vec3 target_pos {};
		Creature::Handle target = c_Invalid;
		int awareness = 0;
		int patience = 0;
		int pathfind_ready = 0;
		int last_taunt = c_Invalid;

		int any_attack_range = c_Invalid; // Range of our longest-range attack.
		int dmg_attack_range = c_Invalid; // Range of our most damaging attack.
		int all_attack_range = c_Invalid; // Range of our shortest-range attack.

		Bot::State state = Rest;
	};

	void clear();
	void serialize(ISerializer& s);

	void reset_brain(Creature::Handle creature);

	// Player pathfinding bot
	bool try_player_pathfind(Vec3 goal);
	bool try_player_collect();
	bool try_player_explore();
	bool has_player_path();
	Vec2 pop_player_path(); // returns {0,0} on failure
	void clear_player_path();

	// Enemy creature bot
	void do_turn(Creature::Handle creature_index);

	// Tell a creature it should go take a look over there, if it's not busy.
	void notify_investigate(Creature::Handle creature, Vec3 attack_origin);

	// Tell a creature it was hit by a beam.  Details of beam may be inspected.
	void notify_hit_by_beam(Creature::Handle creature, const Beam::Data& beam_data);

	// Tell a creature its attacks have changed, so recalculate attack ranges.
	void notify_attacks_changed(Creature::Handle creature);
}
