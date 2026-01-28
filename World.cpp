#include "World.h"

#include "Draw.h"
#include "Line.h"
#include "VectorUtil.h"

World s_world;

void World::clear()
{
	s_world = World();
}

World& World::edit()
{
	return s_world;
}

World const& World::read()
{
	return s_world;
}

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

void World::set_visibility(Vec3 pos, Visibility v)
{
	int const map_id = find_map(pos);
	if (map_id != c_invalid)
	{
		maps[map_id].set_visibility(pos.xy(), v);
	}
}

void World::update_visibility(Vec3 viewer, int vision_radius)
{
	// TODO special cases aorund stairs

	// Clear current sight but remember it was seen in the past
	// TODO perhaps we don't need to do every map?
	for (int m = 0; m < maps.size(); ++m)
	{
		maps[m].convert_visible_to_explored();
	}

	// Add new sight along every line in the cache.
	int const num_lines = LineCache::get_num();
	for (int line_id = 0; line_id < num_lines; ++line_id)
	{
		for (LineCache::Itr3D itr(viewer, line_id);
			itr && within_range(viewer, *itr, vision_radius);
			++itr)
		{
			set_visibility(*itr, Visibility::Visible);

			Terrain t = get_terrain(*itr);
			if (!terrain_permits_sight(t))
			{
				break;
			}
		}
	}

	// Hack to add visibility on walls that "should" be visible.
	wall_visibility_hack(viewer, AXIS_X, 1);
	wall_visibility_hack(viewer, AXIS_X, -1);
	wall_visibility_hack(viewer, AXIS_Y, 1);
	wall_visibility_hack(viewer, AXIS_Y, -1);
}

void World::wall_visibility_hack(Vec3 viewer, Axis a, int sign)
{
	for (int r = 6; r <= 7; ++r)
	{
		Vec3 open_pos = viewer;
		open_pos[a] += (r * sign);

		if (get_visibility({ open_pos.x,open_pos.y }) == Visibility::Visible)
		{
			Axis other_axis = get_other_axis(a);
			Vec3 pos1 = open_pos;
			Vec3 pos2 = open_pos;
			pos1[other_axis] += 1;
			pos2[other_axis] -= 1;

			if (!terrain_permits_sight(get_terrain(pos1)))
			{
				set_visibility(pos1, Visibility::Visible);
			}

			if (!terrain_permits_sight(get_terrain(pos2)))
			{
				set_visibility(pos2, Visibility::Visible);
			}
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
