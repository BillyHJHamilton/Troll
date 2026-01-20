#include "BearLibTerminal.h"
#include "Map.h"

#include "Draw.h"
#include "Target.h"

#include <cassert>

static Map global_map;

Map & g_map () { return global_map; }

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

void Map::init(Box const & box, Terrain fill)
{
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

void Map::fill(Terrain t)
{
	for (Vec2 const & pos : map_box)

	{
		assert(contains(pos));
		set_terrain(pos, t);
	}
}

void Map::fill_box(Box const & box, Terrain t)
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

void Map::update_visibility(Vec2 const & viewer, int max_radius)
{
	// remove current sight
	for (Vec2 const & pos : map_box)
	{
		if (get_visibility(pos) == Visibility::Visible)
		{
			set_visibility(pos, Visibility::Explored);
		}
	}

	// add new sight within range
	Vec2 min = {viewer.x - max_radius, viewer.y - max_radius};
	Vec2 size = {2 * max_radius + 1, 2 * max_radius + 1};
	Box vis_box = {min, size};

	for (Vec2 const & pos : vis_box)
	{
		bool within_circle = check_within_range(viewer, pos, max_radius);

		if (within_circle && check_los(*this, viewer, pos))
		{
			set_visibility(pos, Visibility::Visible);
		}
	}
}

void Map::draw_map_tile (Vec2 global_pos, Draw::View const & view, bool ignore_visibility)
{
	Terrain t = get_terrain(global_pos);
	int code = terrain_character(t);

	Visibility v = get_visibility(global_pos);
	if (ignore_visibility || v == Visibility::Visible)
	{
		if (Target::is_target(global_pos))
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
		if (Target::is_target(global_pos))
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
void Map::draw (Draw::View const & view, bool ignore_visibility)
{
	Box draw_area = map_box.intersection(view.view_area());

	for (Vec2 const & map_pos : draw_area)
	{
		// make sure intersection function works right
		assert(contains(map_pos));
		assert(view.contains_global_pos(map_pos));

		draw_map_tile(map_pos, view, ignore_visibility);
	}
}

bool check_los(Map const & map, Vec2 const & p0, Vec2 const & p1)
{
	LineItr itr(p0, p1);
	itr.advance();				// skip starting point
	while (itr.steps_left > 0)	// skip end point
	{
		Terrain t = map.get_terrain(*itr);
		if (!terrain_permits_sight(t))
		{
			return false;
		}
		itr.advance();
	}

	// no obstruction was met
	return true;
}

