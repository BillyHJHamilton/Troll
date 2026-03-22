#pragma once

#include "Creature.h"
#include "Scratch.h"
#include "Types.h"
#include "Geometry.h"

using Vec3TempList = std::vector<Vec3,Scratch<Vec3>>;

namespace Pathfind
{
	struct NeighbourParam
	{
		// Whether to allow moving over creatures.
		bool ignore_creatures = false;

		// Whether to allow moving to a different z-level.
		bool allow_stairs = true;

		// If set, this creature will be treated as a valid move position.
		// Used when trying to pathfind onto another creature for combat.
		Creature::Handle target_creature = Creature::None;

		// How to handle unexplored tiles.
		enum class UnexploredMode : byte
		{
			Default = 0,	// Treat it as its actual terrain, ignoring LOS
			Block,			// Don't allow pathing through unexplored
			Open,			// Treat all unexplored spaces as open
		};
		UnexploredMode unexplored_mode = UnexploredMode::Default;
	};

	struct AstarParam
	{
		int max_cost = 25;
		bool ignore_creatures = false;
		bool allow_unexplored = true;
	};

	struct ExploreParam
	{
		int max_cost = 100;
		bool allow_stairs = false;

		enum class GoalType : byte
		{
			Item,
			Darkness
		};
		GoalType goal = GoalType::Darkness;
	};

	struct FiringPositionParams
	{
		int max_cost = 3;
		int max_range = 8;
	};
	
	struct NearestOpenParam
	{
		int max_cost = 3;
		int num_to_find = 1;
		bool allow_start = false;
		bool allow_stairs = false;
	};

	// Finds spaces that are valid to move to.  Considers stairs.
	void find_open_neighbours(Vec3 pos, NeighbourParam param, Vec3TempList& out);

	// Pathfinds on the global World.  Avoids creatures, except target creature.
	// Returns a backwards path (so pop back to get your next move).
	// Returns empty vector if it doesn't find a path.
	void astar(Vec3 start, Vec3 goal, AstarParam param, std::vector<Vec3>& path_out);

	// Breadth first search for uncollected item or darkness.
	void into_darkness(Vec3 start, ExploreParam param, std::vector<Vec3>& path_out);

	// Breadth first search for an open firing position.
	void find_firing_position(Creature::Handle creature, Vec3 target, FiringPositionParams param,
		std::vector<Vec3>& path_out);

	// Breadth first search for open points near the target.
	void find_nearest_open(Vec3 start, NearestOpenParam param, Vec3TempList& list_out);

}
