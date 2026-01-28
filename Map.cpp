#include "BearLibTerminal.h"
#include "Map.h"

#include "Draw.h"
#include "Line.h"
#include "Random.h"
#include "Target.h"

#include <cassert>
#include <iostream>

//-------------------------------------------------------------------------------------------------
// Terrain

int constexpr SOLID_BLOCK = 9608;

int terrain_character(Terrain t)
{
	switch(t)
	{
	case Terrain::Open: return '.';
	case Terrain::Wall: return SOLID_BLOCK;
	default: assert(false); return '?';
	}
}

bool terrain_permits_sight(Terrain t)
{
	switch(t)
	{
	case Terrain::Open: return true;
	case Terrain::Wall: return false;
	default: assert(false); return false;
	}
}

bool terrain_is_solid(Terrain t)
{
	switch(t)
	{
	case Terrain::Open: return false;
	case Terrain::Wall: return true;
	default: assert(false); return false;
	}
}

//-------------------------------------------------------------------------------------------------
// Map

void Map::init(int z, Box2 const & box, Terrain fill)
{
	global_z = z;
	map_box = box;

	terrain = make_grid(box.size.x, box.size.y, fill);
	visibility = make_grid(box.size.x, box.size.y, Visibility::Hidden);
}

Terrain Map::get_terrain(Vec2 const & global_pos) const
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return terrain[local.x][local.y];
}

Visibility Map::get_visibility(Vec2 const & global_pos) const
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	return visibility[local.x][local.y];
}

void Map::set_terrain(Vec2 const & global_pos, Terrain t)
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	terrain[local.x][local.y] = t;
}

void Map::set_visibility(Vec2 const & global_pos, Visibility v)
{
	Vec2 local = global_to_local(global_pos);
	assert(local_pos_valid(local));
	visibility[local.x][local.y] = v;
}

bool Map::tile_is_solid(Vec2 const & global_pos) const
{
	Terrain t = get_terrain(global_pos);
	return terrain_is_solid(t);
}

bool Map::tile_permits_sight(Vec2 const& global_pos) const
{
	Terrain t = get_terrain(global_pos);
	return terrain_permits_sight(t);
}

void Map::fill(Terrain t)
{
	for (Vec2 const & pos : map_box)

	{
		assert(contains(pos));
		set_terrain(pos, t);
	}
}

void Map::fill_box(Box2 const & box, Terrain t)
{
	assert(contains(box));
	for (Vec2 const & pos : box)
	{
		set_terrain(pos, t);
	}
}

void Map::clear_visibility()
{
	for (Vec2 const & pos : map_box)
	{
		set_visibility(pos, Visibility::Hidden);
	}
}

void Map::update_visibility(Vec2 const & viewer_global, int max_radius)
{
	// Remove current sight.
	for (Vec2 const & pos : map_box)
	{
		if (get_visibility(pos) == Visibility::Visible)
		{
			set_visibility(pos, Visibility::Explored);
		}
	}

	// Add new sight along every line in the cache.
	int const num_lines = LineCache::get_num();
	for (int line_id = 0; line_id < num_lines; ++line_id)
	{
		for (LineCache::Itr itr(viewer_global, line_id);
			itr
			  && within_range(viewer_global, *itr, max_radius)
			  && contains(*itr);
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
	add_wall_visibility(viewer_global, AXIS_X,  1);
	add_wall_visibility(viewer_global, AXIS_X, -1);
	add_wall_visibility(viewer_global, AXIS_Y,  1);
	add_wall_visibility(viewer_global, AXIS_Y, -1);
}

void Map::add_wall_visibility(Vec2 viewer_global, Axis a, int sign)
{
	for (int r = 6; r <= 7; ++r)
	{
		Vec2 open_pos = viewer_global;
		open_pos[a] += (r * sign);
		
		if (contains(open_pos) &&
			visibility[open_pos.x][open_pos.y] == Visibility::Visible)
		{
			Axis other_axis = get_other_axis(a);
			Vec2 pos1 = open_pos;
			Vec2 pos2 = open_pos;
			pos1[other_axis] += 1;
			pos2[other_axis] -= 1;

			if (contains(pos1)
				&& !terrain_permits_sight(get_terrain(pos1)))
			{
				set_visibility(pos1, Visibility::Visible);
			}

			if (contains(pos2)
				&& !terrain_permits_sight(get_terrain(pos2)))
			{
				set_visibility(pos2, Visibility::Visible);
			}
		}
	}
}

void Map::draw_map_tile (Vec2 global_pos, Draw::View const & view, bool ignore_visibility) const
{
	Vec3 const pos3 = global_pos.xyz(global_z);
	const bool is_target = Target::is_target(pos3);

	Terrain const t = get_terrain(global_pos);
	int const code = terrain_character(t);

	Visibility v = get_visibility(global_pos);
	if (ignore_visibility || v == Visibility::Visible)
	{
		if (is_target)
		{
			Draw::draw_tile_bg(code, global_pos, view, "white", TARGET_COLOUR);
		}
		else
		{
			Draw::draw_tile(code, global_pos, view, "white");
		}
	}
	else if (v == Visibility::Explored)
	{
		if (is_target)
		{
			Draw::draw_tile_bg(code, global_pos, view, "darker grey", TARGET_COLOUR);
		}
		else
		{
			Draw::draw_tile(code, global_pos, view, "darker grey");
		}
	}
}

// viewport - box on the screen (in wide tiles) where the map will be drawn
// start - upper left position of the map to draw
void Map::draw (Draw::View const & view, bool ignore_visibility) const
{
	if (global_z != view.z)
	{
		// TODO special cases around staircases
		return;
	}

	Box2 draw_area = map_box.intersection(view.view_area());

	for (Vec2 const & map_pos : draw_area)
	{
		// make sure intersection function works right
		assert(contains(map_pos));
		assert(view.contains_global_pos(map_pos));

		draw_map_tile(map_pos, view, ignore_visibility);
	}
}

void Map::test_los_symmetry()
{
	std::cout << "LOS symmetry test: \n";

	int constexpr tests = 200;
	int errors = 0;
	for (int i = 0; i < tests; ++i)
	{
		Vec2 p0 = Random::in_box(map_box);
		Vec2 p1;
		do
		{
			p1 = Random::in_box(map_box);
		}
		while (p0 == p1 || !within_range(p0, p1, 8));

		bool los_0_to_1 = has_los(*this, p0, p1);
		bool los_1_to_0 = has_los(*this, p1, p0);

		if (los_0_to_1 != los_1_to_0)
		{
			++errors;
			std::cout << "- Error: Line from (" << p0.x << ", " << p0.y
				<< ") to (" << p1.x << ", " << p1.y << ") is "
				<< ((los_0_to_1) ? "open" : "blocked") << " but reverse is "
				<< ((los_1_to_0) ? "open" : "blocked") << ".\n";

			for (BoxItr itr(map_box); itr; ++itr)
			{
				if (*itr == p0)
				{
					std::cout << '0';
				}
				else if (*itr == p1)
				{
					std::cout << '1';
				}
				else if (!terrain_permits_sight(get_terrain(*itr)))
				{
					std::cout << '=';
				}
				else
				{
					std::cout << '.';
				}

				if (itr->x == map_box.inner_max(0))
				{
					std::cout << '\n';
				}
			}

			// Break here to debug.
			if (los_0_to_1)
			{
				bool retry_0_to_1 = has_los(*this, p0, p1);
			}
			else
			{
				bool retry_1_to_0 = has_los(*this, p0, p1);
			}
		}
	}

	std::cout << errors << " errors out of " << tests << " tests.\n";
}

// helper function
bool has_los_on_line(Map const& map, Vec2 p0, Vec2 p1, int line_id)
{
	LineCache::Itr itr(p0, line_id);
	itr.advance();            // skip start point
	while (itr && *itr != p1 && map.local_pos_valid(*itr)) // skip end point
	{
		Terrain t = map.get_terrain(*itr);
		if (!terrain_permits_sight(t))
		{
			return false;
		}
		itr.advance();
	}
	return true;
}

int get_los(Map const& map, Vec2 const& p0, Vec2 const& p1)
{
	std::vector<int> const& lines = LineCache::get_lines(p0, p1);
	for (int line_id : lines)
	{
		if (has_los_on_line(map, p0, p1, line_id))
		{
			return line_id;
		}
	}

	// No open line was found.
	return c_invalid;
}

bool has_los(Map const& map, Vec2 const& p0, Vec2 const& p1)
{
	return get_los(map, p0, p1) != c_invalid;
}
