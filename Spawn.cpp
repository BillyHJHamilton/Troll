#include "Spawn.h"

#include "Creature.h"
#include "Debug.h"
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

std::vector<bool> s_spawned;

// Declared statically here to avoid needless reallocations.
std::vector<Vec2> s_spawn_positions;

//-----------------------------------------------------------------------------
// Helpers

void find_spawn_positions(const Map& map)
{
	s_spawn_positions.clear();

	for (BoxItr itr(map.get_box()); itr; ++itr)
	{
		Vec2 const pos2 = *itr;
		Vec3 const pos3 = itr->xyz(map.get_z());

		if (map.get_terrain(pos2) == Terrain::Open &&
			!map.has_item(pos2) &&
			!World::read().is_visible(pos3) &&
			Creature::creature_at_pos(pos3) == Creature::None)
		{
			s_spawn_positions.push_back(pos2);
		}
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

// Call find_spawn_positions first.
void spawn_creatures(Map const& map, int creatures_to_spawn)
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
			std::cout << std::format(" - Spawned {} (difficulty {}) at ({},{}).\n",
				creature.long_name(), Gingerbread::read(creature.type()).difficulty,
				creature.pos().x, creature.pos().y);
		}

		++creatures_spawned;
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Spawned {}/{} creatures.\n",
			creatures_spawned, creatures_to_spawn);
	}
}

void spawn_items(Map const& map, int items_to_spawn)
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
}

//-----------------------------------------------------------------------------
// Interface

void clear()
{
	s_spawned.clear();
}

void post_world_setup()
{
	int const num_maps = World::read().num_maps();
	Util::Fill(s_spawned, num_maps, false);
}

void check_spawning()
{
	int const map_id = World::read().find_map(Player::pos());

	if (Util::IsValidIndex(s_spawned, map_id) && !s_spawned[map_id])
	{
		Map const& map = World::read().read_map(map_id);

		if (c_ShowMapDebug)
		{
			std::cout << std::format("\nSpawning for map {}, difficulty {}.\n",
				map_id, map.get_difficulty());
		}

		find_spawn_positions(map);

		int const creatures_to_spawn = Random::in_range(4,6);
		spawn_creatures(map, creatures_to_spawn);

		int const items_to_spawn = Random::in_range(20,40);
		spawn_items(map, items_to_spawn);

		s_spawned[map_id] = true;
	}
}

} // namespace Spawn
