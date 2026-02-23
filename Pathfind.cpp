#include "Pathfind.h"

#include "Creature.h"
#include "Debug.h"
#include "MapUtil.h"
#include "PerfTimer.h"
#include "Scratch.h"
#include "Stairs.h"
#include "World.h"

#include <functional> // std::greater
#include <queue> // for priority queue :*(
#include <unordered_map>

namespace Pathfind
{

// Count each vertical step as this many horizontal in heuristic function,
// since usually it will be necessary to get onto the right vertical level
// before it's possible to reach the target.
int constexpr c_HeightFactor = 10;

// I hate the std::priority_queue, but I guess it's okay for this case.
// Wrapper based on the redblob link below.
struct AStarPriorityQueue
{
	using PriorityType = int;
	using PQElement = std::pair<PriorityType, Vec3>;

	struct Comparator
	{
		bool operator()(PQElement lhs, PQElement rhs) { return lhs.first > rhs.first; }
	};

	std::priority_queue<PQElement, std::vector<PQElement,Scratch<PQElement>>, Comparator> elements;

	inline bool empty() const { return elements.empty(); }
	inline void add(Vec3 item, PriorityType priority) { elements.emplace(priority, item); }

	inline Vec3 pop()
	{
		Vec3 best_item = elements.top().second;
		elements.pop();
		return best_item;
	}
};

void find_open_neighbours(Vec3 pos, Vec3TempList& out, Creature::Handle target)
{
	PerfTimer perf0("find_open_neighbours");

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

	{
		PerfTimer perf1("find_open_neighbours - compass loop");
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
}

// Implementation based on www.redblobgames.com/pathfinding/a-star/implementation.html
// "License: all the sample code on this page is free to use in your projects.
//	If you need a license for it, you can treat it as Apache v2 licensed by Red Blob Games."

void astar(Vec3 start, Vec3 goal, int max_cost, std::vector<Vec3>& path_out)
{
	PerfTimer perf0("astar");

	path_out.clear();
	path_out.reserve(max_cost);

	World const& world = World::read();
	Creature::Handle const target = Creature::creature_at_pos(goal);

	if (chessboard_distance(start, goal) > max_cost)
	{
		if (Debug::enabled(Debug::Pathfind))
		{
			std::cout << "Target is beyond max cost.  Pathfinding skipped.\n";
		}

		return;
	}

	struct NodeInfo
	{
		Vec3 come_from;
		int total_cost;
	};
	std::unordered_map<Vec3, NodeInfo, std::hash<Vec3>, std::equal_to<Vec3>,
		Scratch<std::pair<const Vec3,NodeInfo>>> discovered;

	Vec3TempList neighbours; // to avoid reallocating inside loop

	discovered.insert_or_assign(start, NodeInfo 
	{
		start,  // comes from itself, sure.
		0       // by definition.
	});

	AStarPriorityQueue frontier;
	frontier.add(start, 0);

	{
		PerfTimer perf1("astar - core loop");

		while (!frontier.empty())
		{
			PerfTimer perf2("astar - per iteration");

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

					int const vertical_distance = std::abs(neighbour.z - goal.z);
					int const heuristic = manhattan_distance(neighbour.xy(), goal.xy())
						+ c_HeightFactor*vertical_distance;
					frontier.add(neighbour, heuristic);
				}
			}
		}
	}

	NodeInfo const* const goal_node = Util::Find(discovered, goal);
	if (goal_node)
	{
		PerfTimer perf1("astar - rebuild path");

		Vec3 boomerang = goal;
		while (boomerang != start)
		{
			path_out.push_back(boomerang);
			boomerang = discovered[boomerang].come_from;
		}

		if (Debug::enabled(Debug::Pathfind))
		{
			std::cout << "Built path of " << path_out.size() << " steps.\n";
		}
	}
	else
	{
		if (Debug::enabled(Debug::Pathfind))
		{
			std::cout << "Failed to find a path.\n";
		}
	}
}

}
