#pragma once

#include "Creature.h"
#include "Scratch.h"
#include "Types.h"
#include "Geometry.h"

using Vec3TempList = std::vector<Vec3,Scratch<Vec3>>;

namespace Pathfind
{
	enum class GoalType : byte
	{
		Item,
		Darkness
	};

	struct Parameters
	{
		int max_cost = 25;
		bool ignore_creatures = false;
		bool allow_unexplored = true;
	};

	struct ExploreParameters
	{
		int max_cost = 100;
		bool allow_stairs = false;
		GoalType goal = GoalType::Darkness;
	};

	// Finds spaces that are valid to move to.  Considers stairs.
	// Creatures are treated as impassible unless ignore_creatures is true.
	// However, the target creature is treated as passible, if one is provided.
	void find_open_neighbours(Vec3 pos, bool ignore_creatures, Creature::Handle target,
		Vec3TempList& out);

	// Pathfinds on the global World.  Avoids creatures, except target creature.
	// Returns a backwards path (so pop back to get your next move).
	// Returns empty vector if it doesn't find a path.
	void astar(Vec3 start, Vec3 goal, Parameters param, std::vector<Vec3>& path_out);

	// Breadth first search for uncollected item or darkness.
	void into_darkness(Vec3 start, ExploreParameters param, std::vector<Vec3>& path_out);
}
