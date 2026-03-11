#pragma once

#include "Types.h"
#include "VectorUtil.h"

// Deals with placing characters and items in the world.
namespace Spawn
{
	// Creature spawning constants:
	// TODO May be part of parameters, or global parameters, or some such.

	float constexpr c_MaxOverLevel = 2.0f;
	float constexpr c_MaxUnderLevel = 4.0f;

	// Probability is multiplied by this factor for each level over/under target.
	float constexpr c_OverLevelFactor = 0.5f;
	float constexpr c_UnderLevelFactor = 0.75f;

	struct Option
	{
		enum class Type : byte
		{
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
