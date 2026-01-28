#include "World.h"

#include "Draw.h"
#include "Line.h"
#include "VectorUtil.h"

int World::add_map(int z, Box2 box, Terrain fill)
{
	maps.emplace_back();
	maps.back().init(z, box, fill);
	return (int)maps.size() - 1;
}

int World::find_map(Vec3 global_pos) const
{
	if (Util::IsValidIndex(maps, temp_last_map)
		&& maps[temp_last_map].contains(global_pos))
	{
		return temp_last_map;
	}

	for (int m = 0; m < maps.size(); ++m)
	{
		if (maps[m].contains(global_pos))
		{
			temp_last_map = m;
			return m;
		}
	}

	return c_invalid;
}

Terrain World::get_terrain(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_invalid)
	{
		return maps[map_id].get_terrain(pos.xy());
	}

	return Terrain::Wall; // all walls outside the map
}

bool World::is_solid(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_invalid)
	{
		return maps[map_id].tile_is_solid(pos.xy());
	}

	return true; // consider the world outside any map to be solid.
}

bool World::permits_sight(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_invalid)
	{
		return maps[map_id].tile_permits_sight(pos.xy());
	}

	return false; // off the map, it's unsightly
}

Visibility World::get_visibility(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_invalid)
	{
		return maps[map_id].get_visibility(pos.xy());
	}

	return Visibility::Hidden; // out of map, out of sight
}

void World::update_visibility(Vec3 viewer, int vision_radius)
{
	// TODO special cases aorund stairs
	// TODO perhaps a more global algorithm overall

	for (int m = 0; m < maps.size(); ++m)
	{
		if (maps[m].get_z() == viewer.z)
		{
			maps[m].update_visibility(viewer.xy(), vision_radius);
		}
	}
}

int World::get_los(Vec3 start, Vec3 end) const
{
	if (start.z != end.z)
	{
		// Todo: special cases for stairs.
		return c_invalid;
	}

	// Find possible trajectory lines and test each one.
	std::vector<int> const& lines = LineCache::get_lines(start.xy(), end.xy());
	for (int line_id : lines)
	{
		if (has_los_on_line(start, end, line_id))
		{
			return line_id;
		}
	}

	return c_invalid;
}

bool World::has_los_on_line(Vec3 start, Vec3 end, int line_id) const
{
	LineCache::Itr itr(start.xy(), line_id);
	itr.advance();                  // skip start point
	while (itr && *itr != end.xy()) // skip end point
	{
		if (!permits_sight({itr->x, itr->y, start.z}))
		{
			return false;
		}
		itr.advance();
	}

	return true;
}

bool World::has_los(Vec3 start, Vec3 end) const
{
	return (get_los(start, end) != c_invalid);
}

void World::draw(Draw::View view, bool ignore_visibility) const
{
	for (int m = 0; m < maps.size(); ++m)
	{
		if (maps[m].get_z() == view.z)
		{
			maps[m].draw(view, ignore_visibility);
		}
	}
}
