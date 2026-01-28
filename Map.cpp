#include "BearLibTerminal.h"
#include "Map.h"

#include "Draw.h"
#include "Line.h"
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
				else if (!Terrain::permits_sight(get_terrain(*itr)))
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
		Terrain::Type t = map.get_terrain(*itr);
		if (!Terrain::permits_sight(t))
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
