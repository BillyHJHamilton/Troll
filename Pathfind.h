#pragma once

#include "Types.h"
#include "Geometry.h"

namespace Pathfind
{
	// Finds spaces that are not solid and not occupied by any creature.
	// (except target creature, if one is provided).  Considers stairs.
	void find_open_neighbours(Vec3 pos, std::vector<Vec3>& out, Creature::Handle target);

	// Pathfinders on the global World.  Avoids creatures, except target creature.
	// Returns a backwards path (so pop back to get your next move).
	// Returns empty vector if it doesn't find a path.
	std::vector<Vec3> astar(Vec3 start, Vec3 goal, int max_cost);
}
