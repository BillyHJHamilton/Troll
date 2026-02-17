#include "Spawn.h"

#include "Creature.h"
#include "Debug.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

namespace Spawn
{

//-----------------------------------------------------------------------------
// Data

struct Parameters
{
	// Amount of items to spawn.
	int min_items = 20;
	int max_items = 40;

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
};

struct History
{
	int next_spawn_time = 0;
	int creatures_spawned = 0;
	int items_spawned = 0;

	bool has_ever_spawned() const { return next_spawn_time > 0; }
	void serialize(ISerializer& s);
};

std::vector<Spawn::History> s_spawned;

// List of open positions on current level.
// Declared here in static memory to reduce allocations.
std::vector<Vec2> s_spawn_positions;

//-----------------------------------------------------------------------------
// Helper declarations

// Caches a list of open spawn positions for the map.
// Remains valid until called against for another map, or until time passes.
void find_spawn_positions(const Map& map, int min_range_from_player);

// Checks whether there are still cached spawn positions available.
// Call this before calling next_spawn_position().
bool has_spawn_positions();

// Removes and returns a random spawn position from the cached list.
Vec2 next_spawn_position();

// Check if a map meets the conditions to spawn.
bool is_map_ready(int map_id, int player_map, Parameters const& param);

// Do spawning for a single map.
void spawn_for_map(Map const& map, History& history, Parameters const& param);

// Note: Must call find_spawn_positions first.
int spawn_creatures(Map const& map, int creatures_to_spawn);
int spawn_items(Map const& map, int items_to_spawn);

//-----------------------------------------------------------------------------
// Interface

void clear()
{
	s_spawned.clear();
}

void History::serialize(ISerializer& s)
{
	s.srz_int(next_spawn_time);
	s.srz_int(creatures_spawned);
	s.srz_int(items_spawned);
}

void serialize(ISerializer& s)
{
	srz_vec_size(s, s_spawned);
	for (History& history : s_spawned)
	{
		history.serialize(s);
	}
}

void post_world_setup()
{
	int const num_maps = World::read().num_maps();
	Util::Fill(s_spawned, num_maps, Spawn::History{});
}

void check_spawning()
{
	int const player_map = World::read().find_map(Player::pos());
	if (!Util::IsValidIndex(s_spawned, player_map))
	{
		return;
	}

	// For now, always use the default parameters.
	Parameters const param{};

	for (int map_id = 0; map_id < World::read().num_maps(); ++map_id)
	{
		if (is_map_ready(map_id, player_map, param))
		{
			Map const& map = World::read().read_map(map_id);
			History& history = s_spawned[map_id];

			if (c_ShowMapDebug)
			{
				std::cout << std::format("\nSpawning for map {} at difficulty level {}.\n",
					map_id, map.get_difficulty());
			}

			spawn_for_map(map, history, param);
		}
	}
}

//-----------------------------------------------------------------------------
// Helper Implementations


// Caches a list of open spawn positions for the map.
// Remains valid until called against for another map, or until time passes.
void find_spawn_positions(const Map& map, int min_range_from_player)
{
	s_spawn_positions.clear();

	int num_not_open = 0;
	int num_has_item = 0;
	int num_visible = 0;
	int num_near_player = 0;
	int num_creature = 0;

	for (BoxItr itr(map.get_box()); itr; ++itr)
	{
		Vec2 const pos2 = *itr;
		Vec3 const pos3 = itr->xyz(map.get_z());

		if (map.get_terrain(pos2) != Terrain::Open)
		{
			++num_not_open;
			continue;
		}

		if (map.has_item(pos2))
		{
			++num_has_item;
			continue;
		}

		if (World::read().is_visible(pos3))
		{
			++num_visible;
			continue;
		}

		if (Player::pos().z == map.get_z() &&
			chessboard_distance(Player::pos().xy(), pos2) < min_range_from_player)
		{
			++num_near_player;
			continue;
		}

		if (Creature::creature_at_pos(pos3) != Creature::None)
		{
			++num_creature;
			continue;
		}

		s_spawn_positions.push_back(pos2);
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Found {} valid spawn positions.\n"
			" + {} not open, {} with item, {} visible, {} near player, {} with creature.\n",
			Util::Size(s_spawn_positions), num_not_open, num_has_item, num_visible,
			num_near_player, num_creature);
	}
}

bool has_spawn_positions()
{
	return !s_spawn_positions.empty();
}

Vec2 next_spawn_position()
{
	int const i = Random::index(s_spawn_positions);
	Vec2 const pos = s_spawn_positions.at(i);
	Util::RemoveSwap(s_spawn_positions, i);
	return pos;
}

bool is_map_ready(int map_id, int player_map, Parameters const& param)
{
	const Spawn::History& history = s_spawned[map_id];

	// Spawning doesn't start for a map until visited by player.
	if (!history.has_ever_spawned() && map_id != player_map)
	{
		return false;
	}

	if (history.creatures_spawned >= param.max_creatures ||
		Game::get_turn_number() < history.next_spawn_time)
	{
		// Map is done spawning, or on cooldown.
		return false;
	}

	return true;
}

void spawn_for_map(Map const& map, History& history, Parameters const& param)
{
	bool const is_first_spawn = (history.next_spawn_time == 0);

	int const creatures_to_spawn = is_first_spawn ?
		Random::in_range(param.min_creatures, param.max_creatures) :
		1;
	int const items_to_spawn = is_first_spawn ?
		Random::in_range(param.min_items, param.max_items) :
		0;
	int const min_range = is_first_spawn ? 1 : param.min_range_from_player;

	find_spawn_positions(map, min_range);

	history.creatures_spawned += spawn_creatures(map, creatures_to_spawn);

	if (items_to_spawn > 0)
	{
		history.items_spawned += spawn_items(map, items_to_spawn);
	}

	// Set next cooldown time.
	int const cooldown = Random::in_range(param.cooldown_min, param.cooldown_max);
	history.next_spawn_time = Game::get_turn_number() + cooldown;
}

// Note: Must call find_spawn_positions first.
int spawn_creatures(Map const& map, int creatures_to_spawn)
{
	float const difficulty = map.get_difficulty();
	int creatures_spawned = 0;
	while (has_spawn_positions() && creatures_spawned < creatures_to_spawn)
	{
		Vec2 const pos = next_spawn_position();
		Vec3 const pos3 = pos.xyz(map.get_z());

		Creature::Type type = Gingerbread::find_type_to_spawn(map.get_difficulty());
		if (type == Creature::None)
		{
			break;
		}

		Creature::Handle creature = Creature::spawn_creature(type, pos3);
		if (c_ShowMapDebug)
		{
			std::cout << std::format(" - Spawned {} at ({},{}) - diff {}.\n",
				creature.long_name(), creature.pos().x, creature.pos().y,
				Gingerbread::read(creature.type()).difficulty);
		}

		++creatures_spawned;
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Spawned {}/{} creatures.\n",
			creatures_spawned, creatures_to_spawn);
	}

	return creatures_spawned;
}

// Must call find_spawn_positions first.
int spawn_items(Map const& map, int items_to_spawn)
{
	float const difficulty = map.get_difficulty();
	int items_spawned = 0;
	while (has_spawn_positions() && items_spawned < items_to_spawn)
	{
		Vec2 const pos = next_spawn_position();
		Vec3 const pos3 = pos.xyz(map.get_z());

		if (Random::one_in(11))
		{
			Item::spawn_potion_by_level(pos3, difficulty);
		}
		else
		{
			Item::spawn_bbb(pos3);
		}
		++items_spawned;
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Placed {}/{} items.\n",
			items_spawned, items_to_spawn);
	}

	return items_spawned;
}

} // namespace Spawn
