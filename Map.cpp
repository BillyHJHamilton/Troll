#include "Map.h"

#include "Cloud.h"
#include "Creature.h"
#include "Draw.h"
#include "Line.h"
#include "MapUtil.h"
#include "MapGenerator.h"
#include "Random.h"
#include "Crosshair.h"
#include "Terrain.h"
#include "Visibility.h"

#include <cassert>
#include <iostream>

void Map::init(int z, float map_difficulty, Box2 box, Terrain::Type fill)
{
	global_z = z;
	difficulty = map_difficulty;
	map_box = box;

	terrain = Grid(box.size.x, box.size.y, fill);
	visibility = Grid(box.size.x, box.size.y, c_Invalid);
	clouds = Grid(box.size.x, box.size.y, Cloud::Type::None);
	items = Grid(box.size.x, box.size.y, (Item::Handle)c_Invalid);
}

void Map::serialize(ISerializer& s)
{
	s.srz_box2(map_box);
	s.srz_int(global_z);
	s.srz_string(name);
	s.srz_float(difficulty);

	s.srz_grid(terrain, "map.terrain");
	s.srz_grid(visibility, "map.visibility");
	s.srz_grid_compress(clouds, Cloud::None, "map.clouds");
	s.srz_grid_compress(items, (Item::Handle)c_Invalid, "map.items");

	s.srz_hashmap(cloud_lifetimes, "map.cloud_lifetimes");
	s.srz_hashmap(stairs, "map.stairs");

	s.srz_value(spawn_param);

	bool has_generator = (generator != nullptr);
	s.srz_bool(has_generator);

	if (has_generator)
	{
		get_generator().Serialize(s);
	}

	suggestions.serialize(s);
}

void Map::set_name(std::string&& new_name)
{
	name = new_name;
}

MapGenerator& Map::get_generator()
{
	if (!generator)
	{
		generator = std::make_shared<MapGenerator>(*this);
	}

	return *generator;
}

Terrain::Type Map::get_terrain(Vec2 global_pos) const
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return terrain.read(local.x, local.y);
}

Visibility Map::get_visibility(Vec2 global_pos, int current_step) const
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	int const vis = visibility.read(local.x, local.y);
	if (vis == current_step)
	{
		return Visibility::Visible;
	}
	else if (vis >= 0)
	{
		return Visibility::Explored;
	}
	else
	{
		return Visibility::Hidden;
	}
}

Cloud::Type Map::get_cloud(Vec2 global_pos) const
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return clouds.read(local.x, local.y);
}

int Map::get_cloud_lifetime(Vec2 global_pos) const
{
	if (int const* lifetime = Util::Find(cloud_lifetimes, global_pos))
	{
		return *lifetime;
	}
	return 0;
}

void Map::set_terrain(Vec2 global_pos, Terrain::Type t)
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	terrain.edit(local.x, local.y) = t;
}

void Map::set_visibility(Vec2 global_pos, Visibility v, int current_step)
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	if (v == Visibility::Visible)
	{
		visibility.edit(local.x, local.y) = current_step;
	}
	else if (v == Visibility::Explored)
	{
		assert(current_step > 0);
		visibility.edit(local.x, local.y) = 0;
	}
	else
	{
		visibility.edit(local.x, local.y) = c_Invalid;
	}
}

void Map::set_all_explored()
{
	for (BoxItr itr(map_box); itr; ++itr)
	{
		// using current step 1 because it's not important for Explored
		set_visibility(*itr, Visibility::Explored, 1);
	}
}

bool Map::try_add_cloud(Vec2 global_pos, Cloud::Type cloud, int lifetime)
{
	if (lifetime > 0)
	{
		Vec2 const local = global_to_local(global_pos);
		assert(local_pos_valid(local));

		auto existing_itr = cloud_lifetimes.find(local);
		if (existing_itr != cloud_lifetimes.end())
		{
			if (existing_itr->second > lifetime)
			{
				return false; // failed to add
			}
		}

		clouds.edit(local.x, local.y) = cloud;
		cloud_lifetimes.insert_or_assign(global_pos, lifetime);
		return true;
	}
	return false;
}

void Map::clear_cloud(Vec2 global_pos)
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	clouds.edit(local.x, local.y) = Cloud::None;
}

void Map::step_clouds()
{
	for (auto itr = cloud_lifetimes.begin(); itr != cloud_lifetimes.end(); )
	{
		Vec2 const pos = itr->first;
		int& lifetime = itr->second;

		Cloud::Type cloud_type = clouds.read(pos.x, pos.y);
		if (Cloud::affects_creatures(cloud_type))
		{
			Creature::Handle creature = Creature::creature_at_pos(pos.xyz(global_z));
			if (creature.valid())
			{
				Cloud::affect_creature(cloud_type, creature);
			}
		}

		--lifetime;
		if(lifetime <= 0)
		{
			clear_cloud(pos);
			itr = cloud_lifetimes.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

void Map::clear_clouds()
{
	for (auto & pair : cloud_lifetimes)
	{
		clear_cloud(pair.first);
	}
	cloud_lifetimes.clear();
}

bool Map::has_item(Vec2 global_pos) const
{
	return peek_item(global_pos) != c_Invalid;
}

Item::Handle const Map::peek_item(Vec2 global_pos) const
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return items.read(local.x, local.y);
}

void Map::add_item(Vec2 global_pos, Item::Handle item)
{
	assert(item.valid());
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	Item::Handle item_on_floor = items.read(local.x, local.y);
	item.stack_onto(item_on_floor);
	items.edit(local.x, local.y) = item;
}

Item::Handle Map::pop_item(Vec2 global_pos)
{
	Vec2 const local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return Item::unstack(items.edit(local.x, local.y));
}

bool Map::tile_is_solid(Vec2 global_pos) const
{
	Terrain::Type t = get_terrain(global_pos);
	return Terrain::is_solid(t);
}

bool Map::tile_permits_sight(Vec2 global_pos) const
{
	Terrain::Type t = get_terrain(global_pos);
	return Terrain::permits_sight(t);
}

void Map::fill(Terrain::Type t)
{
	for (Vec2 const & pos : map_box)

	{
		assert(contains(pos));
		set_terrain(pos, t);
	}
}

void Map::fill_box(Box2 global_box, Terrain::Type t)
{
	assert(contains(global_box));
	for (Vec2 const & pos : global_box)
	{
		set_terrain(pos, t);
	}
}

void Map::add_stairs(Vec2 global_pos, Stairs::Direction dir)
{
	assert(!has_stairs(global_pos));
	set_terrain(global_pos, Stairs::get_terrain(dir));
	set_terrain(global_pos + Stairs::relative_move(dir).xy(), Stairs::get_terrain(dir));
	stairs.emplace(global_pos, dir);
}

Stairs::Direction Map::get_stairs(Vec2 global_pos) const
{
	const Stairs::Direction* ptr = Util::Find(stairs, global_pos);
	if (ptr)
	{
		return *ptr;
	}

	return Stairs::None;
}

bool Map::has_stairs(Vec2 global_pos) const
{
	return get_stairs(global_pos) != Stairs::None;
}

void Map::add_corresponding_stairs(const Map& other)
{
	bool add_up;
	if (other.get_z() == get_z() + 1)
	{
		// Other level is above this one.
		// We will add up stairs corresponding to its down stairs;
		add_up = true;
	}
	else if (other.get_z() == get_z() - 1)
	{
		// Other level is below this one.
		// We will add down stairs corresponding to its up stairs.
		add_up = false;
	}
	else
	{
		// Level is not one z away.  We can do nothing.
		return;
	}

	for (const Stairs::Pair& pair : other.stairs)
	{
		Vec2 const start_pos = pair.first;
		Stairs::Direction const dir = pair.second;
		if (add_up != Stairs::is_up(dir))
		{
			Vec2 this_end = start_pos + Stairs::relative_move(dir).xy();
			if (contains(this_end))
			{
				add_stairs(this_end, Stairs::reverse(dir));
			}
		}
	}
}

void Map::remove_stairs(Vec2 global_pos)
{
	Stairs::Direction dir = get_stairs(global_pos);
	if (dir != Stairs::None)
	{
		stairs.erase(global_pos);
		set_terrain(global_pos, Terrain::Wall);
		set_terrain(global_pos + Stairs::relative_move(dir).xy(), Terrain::Wall);
	}
}

void Map::clear_visibility(int current_step)
{
	for (Vec2 const & pos : map_box)
	{
		set_visibility(pos, Visibility::Hidden, current_step);
	}
}

void Map::clean_explored_values(int current_step)
{
	for (Vec2 const& pos : map_box)
	{
		if (get_visibility(pos, current_step) == Visibility::Visible)
		{
			set_visibility(pos, Visibility::Explored, current_step);
		}
	}
}

// helper function
bool has_los_on_line(Map const& map, Vec2 p0, Vec2 p1, int line_id)
{
	LineCache::Itr itr(p0, line_id);
	itr.advance();            // skip start point
	while (itr && *itr != p1 && map.local_pos_valid(*itr)) // skip end point
	{
		Terrain::Type t = map.get_terrain(*itr);
		if (!Terrain::permits_sight(t))
		{
			return false;
		}
		itr.advance();
	}
	return true;
}
