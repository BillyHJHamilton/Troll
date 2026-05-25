#include "Pathfind.h"

#include "Action.h"
#include "Creature.h"
#include "Debug.h"
#include "MapUtil.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Scratch.h"
#include "Stairs.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "Visibility.h"
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

// I don't love the std::priority_queue, but I guess it's okay for this case.
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

void find_open_neighbours(Vec3 pos, NeighbourParam param, Vec3TempList& out)
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
	}

	for (CompassItr itr(true); itr; ++itr)
	{
		Vec3 const next_pos = (itr == stairs_compass) ?
			pos + Stairs::relative_move(stairs_dir) :
			pos + itr.get_vec3();

		Visibility const vis = world.get_visibility(next_pos);

		if (vis == Visibility::Hidden &&
			param.unexplored_mode == UnexploredMode::Block)
		{
			continue;
		}

		if (vis == Visibility::Visible && !param.allow_visible)
		{
			continue;
		}

		Terrain::Type const t = world.get_terrain(next_pos);

		if (!param.allow_stairs && next_pos.z != pos.z)
		{
			continue;
		}

		if (Terrain::is_solid(t) &&
			(vis != Visibility::Hidden || param.unexplored_mode != UnexploredMode::Open))
		{
			continue;
		}

		if (param.creature_mode == CreatureMode::AvoidAll ||
			(param.creature_mode == CreatureMode::AvoidVisible && vis == Visibility::Visible))
		{
			Creature::Handle creature = Creature::creature_at_pos(next_pos);
			if (creature.valid() && creature != param.target_creature)
			{
				continue;
			}
		}

		out.push_back(next_pos);
	}
}

// Implementation based on www.redblobgames.com/pathfinding/a-star/implementation.html
// "License: all the sample code on this page is free to use in your projects.
//	If you need a license for it, you can treat it as Apache v2 licensed by Red Blob Games."

void astar(Vec3 start, Vec3 goal, AstarParam param, std::vector<Vec3>& path_out)
{
	PerfTimer perf0("astar");

	path_out.clear();
	path_out.reserve(param.max_cost);

	World const& world = World::read();
	Creature::Handle const target = Creature::creature_at_pos(goal);

	if (chessboard_3d(start, goal) > param.max_cost)
	{
		if (Debug::enabled(Debug::Bot))
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

	while (!frontier.empty())
	{
		PerfTimer perf2("astar - per iteration");

		Vec3 const here = frontier.pop();
		NodeInfo& here_node = discovered.at(here);

		if (here == goal)
		{
			break;
		}

		NeighbourParam neighbour_param
		{
			.allow_stairs = true,
			.target_creature = target,
			.creature_mode = param.creature_mode,
			.unexplored_mode = param.unexplored_mode
		};

		find_open_neighbours(here, neighbour_param, neighbours);
		for (Vec3 neighbour : neighbours)
		{
			int const new_cost = here_node.total_cost + 1; // for now, no terrain costs

			if (new_cost > param.max_cost)
			{
				continue;
			}

			// Option to prevent pathing through unknown tiles.
			if (param.unexplored_mode == UnexploredMode::Block &&
				neighbour != goal &&
				world.get_visibility(neighbour) == Visibility::Hidden)
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
				int const heuristic = manhattan(neighbour.xy(), goal.xy())
					+ c_HeightFactor*vertical_distance;
				frontier.add(neighbour, heuristic);
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
	}
}

// helper function
bool is_goal(Vec3 pos, ExploreParam::GoalType goal_type)
{
	switch (goal_type)
	{
		case ExploreParam::GoalType::Item:
			return World::read().has_item(pos) && World::read().is_visible(pos);
		case ExploreParam::GoalType::Darkness:
			return World::read().get_visibility(pos) == Visibility::Hidden;
	}
	DebugBreak();
	return false;
}

void into_darkness(Vec3 start, ExploreParam param, std::vector<Vec3>& path_out)
{
	PerfTimer perf0("into_darkness");

	path_out.clear();
	path_out.reserve(param.max_cost);

	World const& world = World::read();

	struct NodeInfo
	{
		Vec3 come_from;
		int total_cost;
	};
	std::unordered_map<Vec3, NodeInfo, std::hash<Vec3>, std::equal_to<Vec3>,
		Scratch<std::pair<const Vec3,NodeInfo>>> discovered;

	Vec3TempList neighbours; // to avoid reallocating inside loop
	Vec3 destination = start; // set if goal is found

	discovered.insert_or_assign(start, NodeInfo 
	{
		start,  // comes from itself, sure.
		0       // by definition.
	});

	// TODO technically we don't need a priority queue here - we could just use a normal queue.
	AStarPriorityQueue frontier;
	frontier.add(start, 0);

	while (!frontier.empty())
	{
		PerfTimer perf2("into_darkness - per iteration");

		Vec3 const here = frontier.pop();
		NodeInfo& here_node = discovered.at(here);

		if (is_goal(here, param.goal))
		{
			destination = here;
			break;
		}

		NeighbourParam neighbour_param
		{
			.allow_stairs = param.allow_stairs,
			.creature_mode = CreatureMode::AvoidVisible,
			.unexplored_mode = (param.goal == ExploreParam::GoalType::Darkness) ?
				UnexploredMode::Open :
				UnexploredMode::Block
		};
		find_open_neighbours(here, neighbour_param, neighbours);
		for (Vec3 neighbour : neighbours)
		{
			int const new_cost = here_node.total_cost + 1; // for now, no terrain costs

			if (new_cost > param.max_cost)
			{
				continue;
			}

			NodeInfo* const old_node = Util::Find(discovered, neighbour);
			if (old_node) // it's BFS, so no need to revisit
			{
				continue;
			}
			else
			{
				discovered[neighbour] = {here, new_cost};
				frontier.add(neighbour, new_cost);
			}
		}
	}

	if (destination != start)
	{
		assert(discovered.contains(destination));
		PerfTimer perf1("into_darkness - rebuild path");

		Vec3 boomerang = destination;

		// Don't actually try to walk onto a wall
		if (world.is_solid(destination))
		{
			boomerang = discovered[boomerang].come_from;
		}

		while (boomerang != start)
		{
			path_out.push_back(boomerang);
			boomerang = discovered[boomerang].come_from;
		}
	}
}

void find_firing_position(Creature::Handle creature, Vec3 target, FiringPositionParams param,
	std::vector<Vec3>& path_out)
{
	PerfTimer perf0("find_firing_position");

	path_out.clear();
	path_out.reserve(param.max_cost);

	World const& world = World::read();

	struct NodeInfo
	{
		Vec3 come_from;
		int total_cost;
	};
	std::unordered_map<Vec3, NodeInfo, std::hash<Vec3>, std::equal_to<Vec3>,
		Scratch<std::pair<const Vec3,NodeInfo>>> discovered;

	Vec3TempList neighbours; // to avoid reallocating inside loop
	Vec3 start = creature.pos();
	Vec3 destination = start; // set if goal is found

	discovered.insert_or_assign(start, NodeInfo 
	{
		start,  // comes from itself, sure.
		0       // by definition.
	});

	// TODO technically we don't need a priority queue here - we could just use a normal queue.
	AStarPriorityQueue frontier;
	frontier.add(start, 0);

	while (!frontier.empty())
	{
		PerfTimer perf2("find_firing_position - per iteration");

		Vec3 const here = frontier.pop();
		NodeInfo& here_node = discovered.at(here);

		if (Action::is_clear_firing_position(creature, here, target, param.max_range))
		{
			destination = here;
			break;
		}

		NeighbourParam neighbour_param
		{
			.allow_stairs = true,
			.creature_mode = CreatureMode::AvoidAll,
		};
		find_open_neighbours(here, neighbour_param, neighbours);
		Random::shuffle_vector(neighbours); // don't always move a predictable direction
		for (Vec3 neighbour : neighbours)
		{
			int const new_cost = here_node.total_cost + 1; // for now, no terrain costs

			if (new_cost > param.max_cost)
			{
				continue;
			}

			NodeInfo* const old_node = Util::Find(discovered, neighbour);
			if (old_node) // it's BFS, so no need to revisit
			{
				continue;
			}
			else
			{
				discovered[neighbour] = {here, new_cost};
				frontier.add(neighbour, new_cost);
			}
		}
	}

	if (destination != start)
	{
		assert(discovered.contains(destination));

		Vec3 boomerang = destination;

		while (boomerang != start)
		{
			path_out.push_back(boomerang);
			boomerang = discovered[boomerang].come_from;
		}
	}
}

void find_nearest_open(Vec3 start, NearestOpenParam param, Vec3TempList& list_out)
{
	PerfTimer perf0("find_nearest_open");

	list_out.clear();
	list_out.reserve(param.num_to_find);

	World const& world = World::read();

	std::unordered_map<Vec3, int, std::hash<Vec3>, std::equal_to<Vec3>,
		Scratch<std::pair<const Vec3,int>>> discovered_cost;

	Vec3TempList neighbours; // to avoid reallocating inside loop

	discovered_cost.insert_or_assign(start, 0);

	if (param.allow_start)
	{
		Terrain::Type const start_terrain = World::read().get_terrain(start);
		if (!Terrain::is_solid(start_terrain) &&
			(param.allow_stairs || !Terrain::is_stairs(start_terrain)) &&
			!Creature::creature_at_pos(start).valid())
		{
			list_out.push_back(start);
			if (Util::Size(list_out) >= param.num_to_find)
			{
				// That was easy.
				return;
			}
		}
	}
		

	// TODO technically we don't need a priority queue here - we could just use a normal queue.
	AStarPriorityQueue frontier;
	frontier.add(start, 0);

	while (!frontier.empty())
	{
		PerfTimer perf2("find_nearest_open - per iteration");

		Vec3 const here = frontier.pop();
		int const here_cost = discovered_cost.at(here);

		NeighbourParam const neighbour_param
		{
			.allow_stairs = param.allow_stairs,
			.allow_visible = param.allow_visible
		};
		find_open_neighbours(here, neighbour_param, neighbours);
		Random::shuffle_vector(neighbours);
		for (Vec3 neighbour : neighbours)
		{
			int const new_cost = here_cost + 1;
			if (new_cost > param.max_cost)
			{
				continue;
			}

			if (discovered_cost.contains(neighbour))
			{
				continue;
			}

			list_out.push_back(neighbour);
			if (Util::Size(list_out) >= param.num_to_find)
			{
				return;
			}

			discovered_cost[neighbour] = new_cost;
			frontier.add(neighbour, new_cost);
		}
	}
}

}
