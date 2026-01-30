#include "BearLibTerminal.h"
#include "Map.h"

#include "Draw.h"
#include "Line.h"
#include "MapUtil.h"
#include "Random.h"
#include "Target.h"
#include "Terrain.h"

#include <cassert>
#include <iostream>

void Map::init(int z, Box2 const & box, Terrain::Type fill)
{
	global_z = z;
	map_box = box;

	terrain = make_grid(box.size.x, box.size.y, fill);
	visibility = make_grid(box.size.x, box.size.y, c_invalid);
}

Terrain::Type Map::get_terrain(Vec2 const & global_pos) const
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return terrain[local.x][local.y];
}

Visibility Map::get_visibility(Vec2 const & global_pos, int current_step) const
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	int const vis = visibility[local.x][local.y];
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

void Map::set_terrain(Vec2 const & global_pos, Terrain::Type t)
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	terrain[local.x][local.y] = t;
}

void Map::set_visibility(Vec2 const & global_pos, Visibility v, int current_step)
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	if (v == Visibility::Visible)
	{
		visibility[local.x][local.y] = current_step;
	}
	else if (v == Visibility::Explored)
	{
		assert(current_step > 0);
		visibility[local.x][local.y] = 0;
	}
	else
	{
		visibility[local.x][local.y] = c_invalid;
	}
}

bool Map::tile_is_solid(Vec2 const & global_pos) const
{
	Terrain::Type t = get_terrain(global_pos);
	return Terrain::is_solid(t);
}

bool Map::tile_permits_sight(Vec2 const& global_pos) const
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

void Map::fill_box(Box2 const & global_box, Terrain::Type t)
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

	for (const auto& pair : other.stairs)
	{
		Vec2 const start_pos = pair.first;
		Stairs::Direction const dir = pair.second;
		if (add_up != Stairs::is_up(dir))
		{
			Vec2 this_end = start_pos + Stairs::relative_move(dir).xy();
			if (contains(this_end))
			{
				add_stairs(this_end, Stairs::corresponding_direction(dir));
			}
		}
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
