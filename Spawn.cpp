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
	
std::vector<bool> s_spawned;

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
			std::cout << std::format("\nSpawning for map {}.\n", map_id);
		}

		float const map_level = map.get_difficulty();

		int const num_to_spawn = Random::in_range(4,6);
		int num_spawned = 0;
		for (int i = 0; i < num_to_spawn; ++i)
		{
			// Find spawn position
			// TODO There's definitely a better way to do this.
			// Like get the list of rooms, etc.
			int const attempts = 100;
			for (int a = 0; a < attempts; ++a)
			{
				Vec2 const pos2 = Random::in_box(map.get_box());
				Vec3 const pos3 = pos2.xyz(Player::pos().z);
				if (map.get_terrain(pos2) == Terrain::Open &&
					!World::read().is_visible(pos3) &&
					Creature::creature_at_pos(pos3) == Creature::None)
				{
					// TODO difficulty per map
					Creature::Type type = Gingerbread::find_type_to_spawn(map_level);
					if (type != Creature::None)
					{
						Creature::Handle creature = Creature::spawn_creature(type, pos3);

						if (c_ShowMapDebug)
						{
							std::cout << std::format(" - Spawned {} at ({},{}).\n",
								creature.long_name(), creature.pos().x, creature.pos().y);
						}

						++num_spawned;
					}
					break;
				}
			}
		}

		if (c_ShowMapDebug)
		{
			std::cout << std::format("Spawned {}/{} for map {}.\n",
				num_spawned, num_to_spawn, map_id);
		}

		int const items_to_spawn = Random::in_range(20,40);
		int items_spawned = 0;
		for (int i = 0; i < items_to_spawn; ++i)
		{
			// Find spawn position
			// TODO Repeating code from above...
			// Again, there must be better ways to do all this.
			int const attempts = 100;
			for (int a = 0; a < attempts; ++a)
			{
				Vec2 const pos2 = Random::in_box(map.get_box());
				Vec3 const pos3 = pos2.xyz(Player::pos().z);
				if (map.get_terrain(pos2) == Terrain::Open &&
					!World::read().has_item(pos3) &&
					Creature::creature_at_pos(pos3) == Creature::None)
				{
					if (Random::one_in(10))
					{
						Item::spawn_potion_by_level(pos3, map_level);
					}
					else
					{
						Item::spawn_bbb(pos3);
					}
					++items_spawned;
					break;
				}
			}
		}

		if (c_ShowMapDebug)
		{
			std::cout << std::format("Placed {}/{} items for map {}.\n",
				items_spawned, items_to_spawn, map_id);
		}

		s_spawned[map_id] = true;
	}
}

} // namespace Spawn
