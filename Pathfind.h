#pragma once

#include "Scratch.h"
#include "Types.h"
#include "Geometry.h"

using Vec3TempList = std::vector<Vec3,Scratch<Vec3>>;

namespace Pathfind
{
	// Finds spaces that are not solid and not occupied by any creature.
	// (except target creature, if one is provided).  Considers stairs.
	void find_open_neighbours(Vec3 pos, Vec3TempList& out,
		Creature::Handle target = Creature::None);

	// Pathfinders on the global World.  Avoids creatures, except target creature.
	// Returns a backwards path (so pop back to get your next move).
	// Returns empty vector if it doesn't find a path.
	void astar(Vec3 start, Vec3 goal, int max_cost, std::vector<Vec3>& path_out);
}
