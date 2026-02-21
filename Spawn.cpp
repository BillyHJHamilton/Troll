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

struct History
{
	int next_spawn_time = -1;
	int creatures_spawned = 0;
	int items_spawned = 0;
	int chests_spanwed = 0;

	bool has_ever_spawned() const { return next_spawn_time > -1; }
	void serialize(ISerializer& s);
};

std::vector<Spawn::History> s_spawn_history;

// List of open positions on current level.
// Declared here in static memory to reduce allocations.
std::vector<Vec2> s_spawn_positions;

// Valid positions for treasure chests.  A subset of spawn positions.
std::vector<Vec2> s_special_positions;

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

// Searches remaining spawn positions, so should be called after normal spawning.
// Calling next_chest_position removes from main list and chest list.
void find_chest_positions(const Map& map);
bool is_ok_chest_position(const Map& map, Vec2 pos);
bool has_special_positions();
Vec2 next_special_position();

// Check if a map meets the conditions to spawn.
bool is_map_ready(int map_id, int player_map, Parameters const& param);

// Do spawning for a single map.
void spawn_for_map(Map& map, History& history, Parameters const& param);

// Note: Must call find_spawn_positions first.
int spawn_creatures(Map const& map, int creatures_to_spawn);
int spawn_items(Map const& map, int items_to_spawn);
int spawn_chests(Map& map, int chests_to_spawn);

//-----------------------------------------------------------------------------
// Interface

void clear()
{
	s_spawn_history.clear();
}

void History::serialize(ISerializer& s)
{
	s.srz_int(next_spawn_time);
	s.srz_int(creatures_spawned);
	s.srz_int(items_spawned);
}

void serialize(ISerializer& s)
{
	srz_vector_size(s, s_spawn_history, "s_spawn_history");
	for (History& history : s_spawn_history)
	{
		history.serialize(s);
	}
}

void post_world_setup()
{
	int const num_maps = World::read().num_maps();
	Util::Fill(s_spawn_history, num_maps, Spawn::History{});
}

void check_spawning()
{
	int const player_map = World::read().find_map(Player::pos());
	if (!Util::IsValidIndex(s_spawn_history, player_map))
	{
		return;
	}

	// For now, always use the default parameters.
	Parameters const param{};

	for (int map_id = 0; map_id < World::read().num_maps(); ++map_id)
	{
		if (is_map_ready(map_id, player_map, param))
		{
			Map& map = World::edit().edit_map(map_id);
			History& history = s_spawn_history[map_id];

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

bool is_ok_chest_position(const Map& map, Vec2 pos)
{
	CompassDirection constexpr dirs[4] = {
		c_CompassEast, c_CompassNorth, c_CompassWest, c_CompassSouth };

	for (CompassDirection const dir : dirs)
	{
		CompassDirection const l1 = get_counterclockwise(dir);
		CompassDirection const l2 = get_counterclockwise_90(dir);
		CompassDirection const r1 = get_clockwise(dir);
		CompassDirection const r2 = get_clockwise_90(dir);

		Terrain::Type const t_front = map.get_terrain(pos + c_Compass[dir]);
		Terrain::Type const t_back = map.get_terrain(pos - c_Compass[dir]);
		Terrain::Type const t_l1 = map.get_terrain(pos + c_Compass[l1]);
		Terrain::Type const t_l2 = map.get_terrain(pos + c_Compass[l2]);
		Terrain::Type const t_r1 = map.get_terrain(pos + c_Compass[r1]);
		Terrain::Type const t_r2 = map.get_terrain(pos + c_Compass[r2]);

		// We want it against a wall, with open space in front.
		// And not blocking a hallway on either side.
		bool const front_ok = t_front == Terrain::Open;
		bool const back_ok = t_back == Terrain::Wall;
		bool const left_ok = (t_l1 == Terrain::Wall || t_l2 == Terrain::Open) && t_l1 == t_l2;
		bool const right_ok = (t_r1 == Terrain::Wall || t_r2 == Terrain::Open) && t_r1 == t_r2;

		if (front_ok && back_ok && left_ok && right_ok)
		{
			return true;
		}
	}
	return false;
}

// TODO: We don't really want to place chests adjacent.  This is kind of flawed in that respect.
// TODO: Also this should really happen during map gen since it's a form of terrain.
void find_chest_positions(const Map& map)
{
	s_special_positions.clear();

	for (Vec2 v : s_spawn_positions)
	{
		if (is_ok_chest_position(map, v))
		{
			s_special_positions.push_back(v);
		}
	}
}

bool has_special_positions()
{
	return !s_special_positions.empty();
}

Vec2 next_special_position()
{
	int const i = Random::index(s_special_positions);
	Vec2 const pos = s_special_positions.at(i);
	Util::RemoveSwap(s_special_positions, i);
	Util::RemoveSwapFirstMatchingItem(s_spawn_positions, pos);
	return pos;
}

bool is_map_ready(int map_id, int player_map, Parameters const& param)
{
	const Spawn::History& history = s_spawn_history[map_id];

	// Spawning doesn't start for a map until visited by player.
	if (!history.has_ever_spawned())
	{
		return (map_id == player_map);
	}

	if (history.creatures_spawned >= param.max_creatures ||
		Game::get_turn_number() < history.next_spawn_time)
	{
		// Map is done spawning, or on cooldown.
		return false;
	}

	return true;
}

void spawn_for_map(Map& map, History& history, Parameters const& param)
{
	bool const is_first_spawn = !history.has_ever_spawned();

	int creatures_to_spawn = 1;
	int items_to_spawn = 0;
	int chests_to_spawn = 0;
	int min_range = param.min_range_from_player;

	if (is_first_spawn)
	{
		creatures_to_spawn = Random::in_range(param.min_creatures, param.max_creatures);
		items_to_spawn = Random::in_range(param.min_items, param.max_items);
		chests_to_spawn = Random::in_range(param.min_chests, param.max_chests);
		min_range = 2;
	}

	find_spawn_positions(map, min_range);

	history.creatures_spawned += spawn_creatures(map, creatures_to_spawn);

	if (chests_to_spawn > 0)
	{
		find_chest_positions(map);
		history.chests_spanwed += spawn_chests(map, chests_to_spawn);
	}

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

		if (Random::one_in(14))
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

int spawn_chests(Map& map, int chests_to_spawn)
{
	int spawned = 0;
	while (has_special_positions() && spawned < chests_to_spawn)
	{
		Vec2 const pos = next_special_position();
		map.set_terrain(pos, Terrain::Chest);
		++spawned;
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Placed {}/{} chests.\n",
			spawned, chests_to_spawn);
	}

	return spawned;
}

} // namespace Spawn
