#pragma once

#include "Types.h"
#include "VectorUtil.h"

// Deals with placing characters and items in the world.
namespace Spawn
{
	struct Parameters
	{
		// One of this creature must be spawned.
		Creature::Type boss = (Creature::Type)c_Invalid;

		// Amount of creatures to spawn initially.
		int min_creatures = 4;
		int max_creatures = 6;

		// Time delay after each spawn event before spawning another creature.
		// (After the initial burst, each round adds only one creatures at a time.)
		int cooldown_min = 120;
		int cooldown_max = 200;

		// This parameter applies only to later spawns, not the initial ones.
		int min_range_from_player = 15;

		// Total amount of creatures to ever spawn on the map.
		int lifetime_max_creatures = 9;

		// Amount of items to spawn.
		int min_items = 25;
		int max_items = 35;

		// Amount of chests to spawn.
		int min_chests = 1;
		int max_chests = 3;
	};

	// Creature spawning constants:
	// TODO May be part of parameters, or global parameters, or some such.

	float constexpr c_MaxOverLevel = 2.0f;
	float constexpr c_MaxUnderLevel = 4.0f;

	// Probability is multiplied by this factor for each level over/under target.
	float constexpr c_OverLevelFactor = 0.5f;
	float constexpr c_UnderLevelFactor = 0.75f;

	struct Option
	{
		enum Type : byte
		{
			None,
			Creature,
			Squad
		};

		Type type = Type::Creature;
		int index = c_Invalid;
	};
	using OptionTempList = std::vector<Option,Scratch<Option>>;

	void clear();
	void serialize(ISerializer& s);

	void post_world_setup();
	void check_spawning();

	bool difficulty_in_range (float difficulty, float target_difficulty);
	float probability_factor (float difficulty, float target_difficulty);
}
