#include "Pathfind.h"

#include "Creature.h"
#include "MapUtil.h"
#include "Stairs.h"
#include "World.h"

#include <functional> // std::greater
#include <queue> // for priority queue :*(
#include <unordered_map>

namespace Pathfind
{

// I hate the std::priority_queue, but I guess it's okay for this case.
// Wrapper based on the redblob link below.
struct AStarPriorityQueue
{
	typedef int PriorityType;
	typedef std::pair<PriorityType, Vec3> PQElement;

	struct Comparator
	{
		bool operator()(PQElement lhs, PQElement rhs) { return lhs.first > rhs.first; }
	};

	std::priority_queue<PQElement, std::vector<PQElement>, Comparator> elements;

	inline bool empty() const { return elements.empty(); }
	inline void add(Vec3 item, PriorityType priority) { elements.emplace(priority, item); }

	inline Vec3 pop()
	{
		Vec3 best_item = elements.top().second;
		elements.pop();
		return best_item;
	}
};

void find_open_neighbours(Vec3 pos, std::vector<Vec3>& out, Creature::Handle target)
{
	out.clear();
	out.reserve(8);

	World const& world = World::read();

	CompassDirection stairs_compass = c_CompassInvalid;
	Stairs::Direction const stairs_dir = world.get_stairs(pos);

	if (stairs_dir != Stairs::None)
	{
		stairs_compass = Stairs::compass_dir(stairs_dir);
		Vec3 const other_end = pos + Stairs::relative_move(stairs_dir);
		Creature::Handle const creature = Creature::creature_at_pos(other_end);
		if (creature == Creature::None || creature == target)
		{
			out.push_back(other_end);
		}
	}

	for (CompassItr itr(true); itr; ++itr)
	{
		Vec3 const next_pos = pos + itr.get_vec2().xy0();
		if (itr != stairs_compass &&
			!world.is_solid(next_pos))
		{
			Creature::Handle const creature = Creature::creature_at_pos(next_pos);
			if (creature == Creature::None || creature == target)
			{
				out.push_back(next_pos);
			}
		}
	}
}

// Implementation based on www.redblobgames.com/pathfinding/a-star/implementation.html
// "License: all the sample code on this page is free to use in your projects.
//	If you need a license for it, you can treat it as Apache v2 licensed by Red Blob Games."

std::vector<Vec3> astar(Vec3 start, Vec3 goal, int max_cost)
{
	World const& world = World::read();
	Creature::Handle const target = Creature::creature_at_pos(goal);

	if (chessboard_distance(start, goal) > max_cost)
	{
		std::cout << "Target is beyond max cost.  Pathfinding skipped.\n";
		return std::vector<Vec3>();
	}

	struct NodeInfo
	{
		Vec3 come_from;
		int total_cost;
	};
	std::unordered_map<Vec3, NodeInfo> discovered;

	std::vector<Vec3> neighbours; // to avoid reallocating inside loop

	discovered.insert_or_assign(start, NodeInfo 
	{
		start,  // comes from itself, sure.
		0       // by definition.
	});

	AStarPriorityQueue frontier;
	frontier.add(start, 0);

	while (!frontier.empty())
	{
		Vec3 const here = frontier.pop();
		NodeInfo& here_node = discovered.at(here);

		if (here == goal)
		{
			break;
		}

		find_open_neighbours(here, neighbours, target);
		for (Vec3 neighbour : neighbours)
		{
			int const new_cost = here_node.total_cost + 1; // for now, no terrain costs

			if (new_cost > max_cost)
			{
				continue;
			}

			NodeInfo* const old_node = Util::Find(discovered, neighbour);
			if (old_node && old_node->total_cost <= new_cost)
			{
				continue;
			}
			else
			{
				discovered[neighbour] = {here, new_cost};

				int const heuristic = squared_distance(neighbour, goal);
				frontier.add(neighbour, heuristic);
			}
		}
	}

	NodeInfo const* const goal_node = Util::Find(discovered, goal);
	if (goal_node)
	{
		std::vector<Vec3> path;
		path.reserve(manhattan_distance(start, goal));
		Vec3 boomerang = goal;
		while (boomerang != start)
		{
			path.push_back(boomerang);
			boomerang = discovered[boomerang].come_from;
		}

		std::cout << "Built path of " << path.size() << " steps.\n";
		return (path);
	}
	else
	{
		std::cout << "Failed to find a path.\n";
		return std::vector<Vec3>();
	}
}

}
