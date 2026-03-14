#include "World.h"

#include "Colour.h"
#include "Draw.h"
#include "Line.h"
#include "Map.h"
#include "PerfTimer.h"
#include "Serialize.h"
#include "Target.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "Visibility.h"

World s_world;

/*static*/ void World::clear()
{
	s_world = World();
}

/*static*/ World& World::edit()
{
	return s_world;
}

/*static*/ World const& World::read()
{
	return s_world;
}

void World::serialize(ISerializer& s)
{
	if (s.is_load())
	{
		int map_num;
		s.srz_int(map_num);
		maps.clear();
		maps.reserve(map_num);
		for (int i = 0; i < map_num; ++i)
		{
			maps.emplace_back(std::make_shared<Map>());
			maps.back()->serialize(s);
		}
	}
	else
	{
		int map_num = Util::Size(maps);
		s.srz_int(map_num);
		for (std::shared_ptr<Map>& map_ptr : maps)
		{
			map_ptr->serialize(s);
		}
	}

	s.srz_int(visibility_step);

	if (s.is_load())
	{
		temp_last_map = c_Invalid;
	}
}

int World::add_map(int z, float difficulty, Box2 box, Terrain::Type fill)
{
	maps.emplace_back(std::make_shared<Map>());
	maps.back()->init(z, difficulty, box, fill);
	return (int)maps.size() - 1;
}

int World::num_maps() const
{
	return (int)maps.size();
}

Map& World::edit_map(int index)
{
	return *maps.at(index);
}

const Map& World::read_map(int index) const
{
	return *maps.at(index);
}

int World::find_map(Vec3 global_pos) const
{
	if (Util::IsValidIndex(maps, temp_last_map)
		&& read_map(temp_last_map).contains(global_pos))
	{
		return temp_last_map;
	}

	for (int m = 0; m < maps.size(); ++m)
	{
		if (read_map(m).contains(global_pos))
		{
			temp_last_map = m;
			return m;
		}
	}

	return c_Invalid;
}

float World::find_map_difficulty(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_difficulty();
	}

	return 0;
}

Terrain::Type World::get_terrain(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_terrain(pos.xy());
	}

	return Terrain::Wall; // all walls outside the map
}

bool World::is_solid(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).tile_is_solid(pos.xy());
	}

	return true; // outside the map is solid
}

bool World::permits_sight(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).tile_permits_sight(pos.xy());
	}

	return false; // off the map, it's unsightly
}

void World::set_terrain(Vec3 pos, Terrain::Type new_terrain)
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return edit_map(map_id).set_terrain(pos.xy(), new_terrain);
	}
}

bool World::is_choke_point(Vec3 pos) const
{
	if (is_solid(pos))
	{
		return false;
	}

	bool solid [c_CompassNoMove];
	for (int i = 0; i < (int)c_CompassNoMove; ++i)
	{
		solid[i] = is_solid(pos + c_Compass[i].xy0());
	}

	// Obvious hallway/doorway cases:
	// ...  .#.
	// #@#  .@.
	// ...  .#.
	if (solid[c_CompassNorth] && solid[c_CompassSouth])
	{
		return true;
	}
	if (solid[c_CompassEast] && solid[c_CompassWest])
	{
		return true;
	}

	// And the tricky diagonals:
	// ##.  #..  .##  ..#
	// .@#  #@.  #@.  .@#
	// ..#  .##  #..  ##.
	if (solid[c_CompassNorthwest] && solid[c_CompassSoutheast] &&
		((solid[c_CompassNorth] && solid[c_CompassEast]) || 
		 (solid[c_CompassWest] && solid[c_CompassSouth])))
	{
		return true;
	}
	if (solid[c_CompassSouthwest] && solid[c_CompassNortheast] &&
		((solid[c_CompassSouth] && solid[c_CompassEast]) || 
		 (solid[c_CompassWest] && solid[c_CompassNorth])))
	{
		return true;
	}

	return false;
}

Cloud::Type World::get_cloud(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_cloud(pos.xy());
	}

	return Cloud::None; // cloudless infinity
}

int World::get_cloud_lifetime(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_cloud_lifetime(pos.xy());
	}

	return 0;
}

bool World::try_add_cloud(Vec3 pos, Cloud::Type cloud, int lifetime)
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return edit_map(map_id).try_add_cloud(pos.xy(), cloud, lifetime);
	}
	return false;
}

void World::clear_cloud(Vec3 pos)
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return edit_map(map_id).clear_cloud(pos.xy());
	}
}

void World::step_clouds()
{
	for (int m = 0; m < maps.size(); ++m)
	{
		edit_map(m).step_clouds();
	}
}

void World::clear_clouds()
{
	for (int m = 0; m < maps.size(); ++m)
	{
		edit_map(m).clear_clouds();
	}
}

bool World::has_item(Vec3 pos) const
{
	return peek_item(pos) != c_Invalid;
}

Item::Handle const World::peek_item(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).peek_item(pos.xy());
	}
	return c_Invalid;
}

void World::add_item(Vec3 pos, Item::Handle item)
{
	int const map_id = find_map(pos);
	if (Check(map_id != c_Invalid, "Failed to add item to world."))
	{
		edit_map(map_id).add_item(pos.xy(), item);
	}
}

Item::Handle World::pop_item(Vec3 pos)
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return edit_map(map_id).pop_item(pos.xy());
	}
	return c_Invalid;
}

Stairs::Direction World::get_stairs(Vec3 pos) const
{
	PerfTimer perf("get_stairs");

	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_stairs(pos.xy());
	}

	return Stairs::None; // no stairs off the map
}

bool World::has_stairs(Vec3 pos) const
{
	return get_stairs(pos) != Stairs::None;
}

int World::get_stairs_dz(Vec3 old_pos, Vec2 new_pos) const
{
	Stairs::Direction dir = get_stairs(old_pos);
	if (dir != Stairs::None)
	{
		// Check if we moved in the direction of the stairs.
		Vec2 const this_move = new_pos - old_pos.xy();
		Vec3 const stairs_move = Stairs::relative_move(dir);
		if (this_move == stairs_move.xy())
		{
			return stairs_move.z;
		}
	}

	return 0;
}

Visibility World::get_visibility(Vec3 pos) const
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		return read_map(map_id).get_visibility(pos.xy(), visibility_step);
	}

	return Visibility::Hidden; // out of map, out of sight
}

bool World::is_visible(Vec3 pos) const
{
	return get_visibility(pos) == Visibility::Visible;
}

void World::set_visibility(Vec3 pos, Visibility v)
{
	int const map_id = find_map(pos);
	if (map_id != c_Invalid)
	{
		edit_map(map_id).set_visibility(pos.xy(), v, visibility_step);
	}
}

void World::update_visibility(Vec3 viewer, int vision_radius)
{
	PerfTimer perf0("update_visibility");

	// Convert old vision to fog of war.
	advance_visibility_step();

	set_visibility(viewer, Visibility::Visible);

	Box2 visbox =
	{
		viewer.xy() - Vec2{vision_radius, vision_radius},
		Vec2{2*vision_radius + 1, 2*vision_radius + 1}
	};

	for (BoxItr itr(visbox); itr; ++itr)
	{
		Vec3 const target = itr->xyz(viewer.z);
		if (has_los(viewer, target, vision_radius))
		{
			set_visibility(target, Visibility::Visible);
		}
	}

	// EDIT: Unfortunately this approach causes asymmetric LOS.
	// TODO Implement the new RSPCVT algorithm.
	// Check LOS along every line in the cache
	//int const num_lines = LineCache::get_num();
	//for (int line_id = 0; line_id < num_lines; ++line_id)
	//{
	//	LineCache::Itr3D itr(viewer, line_id);
	//	++itr; // skip start pos
	//	for (;
	//		itr && within_range(viewer, *itr, vision_radius);
	//		++itr)
	//	{
	//		set_visibility(*itr, Visibility::Visible);

	//		Terrain::Type t = get_terrain(*itr);
	//		if (!Terrain::permits_sight(t))
	//		{
	//			break;
	//		}
	//	}
	//}

	add_stairs_visibility(viewer);

	// Hack to add visibility on walls that "should" be visible.
	wall_visibility_hack(viewer, AXIS_X, 1);
	wall_visibility_hack(viewer, AXIS_X, -1);
	wall_visibility_hack(viewer, AXIS_Y, 1);
	wall_visibility_hack(viewer, AXIS_Y, -1);
}

void World::add_stairs_visibility(Vec3 viewer)
{
	Stairs::Direction dir = get_stairs(viewer);
	if (dir != Stairs::None)
	{
		Vec3 const stairs_pos = viewer + Stairs::relative_move(dir);
		set_visibility(stairs_pos, Visibility::Visible);
	}
}

void World::wall_visibility_hack(Vec3 viewer, Axis a, int sign)
{
	int constexpr c_MinDist = 3;
	int constexpr c_MaxDist = 7;
	for (int r = c_MinDist; r <= c_MaxDist; ++r)
	{
		Vec3 const open_pos = viewer.adjusted(a, r*sign);

		if (is_visible(open_pos))
		{
			Axis const other_axis = get_other_axis(a);
			Vec3 const p1 = open_pos.adjusted(other_axis, 1);
			Vec3 const p2 = open_pos.adjusted(other_axis, 2);
			Vec3 const m1 = open_pos.adjusted(other_axis, -1);
			Vec3 const m2 = open_pos.adjusted(other_axis, -2);

			if (is_visible(p1))
			{
				if (r >= 5 &&
					is_visible(p2.adjusted(a, -1*sign)) &&
					!permits_sight(p2) &&
					is_solid(p2))
				{
					set_visibility(p2, Visibility::Visible);
				}
			}
			else if (!permits_sight(p1) && is_solid(p1))
			{
				set_visibility(p1, Visibility::Visible);
			}
			
			if (is_visible(m1))
			{
				if (r >= 5 &&
					is_visible(m2.adjusted(a, -1*sign)) &&
					!permits_sight(m2) &&
					is_solid(m2))
				{
					set_visibility(m2, Visibility::Visible);
				}
			}
			else if (!permits_sight(m1) && is_solid(m1))
			{
				set_visibility(m1, Visibility::Visible);
			}
		}
	}
}

void World::advance_visibility_step()
{
	if (visibility_step == INT_MAX)
	{
		for (int m = 0; m < maps.size(); ++m)
		{
			edit_map(m).clean_explored_values(visibility_step);
		}
		visibility_step = 1;
	}
	else
	{
		++visibility_step;
	}
}

int World::get_los(Vec3 start, Vec3 end, int range) const
{
	PerfTimer perf0("get_los");

	if (range != -1 && !within_range(start, end, range))
	{
		return c_Invalid;
	}

	if (start.z != end.z)
	{
		// Special cases for stairs
		Stairs::Direction const dir = get_stairs(start);
		if (dir != Stairs::None &&
			start + Stairs::relative_move(dir) == end)
		{
			return LineCache::c_StairsLine;
		}

		// Without stairs, can't see between floors.
		return c_Invalid;
	}

	// Find possible trajectory lines and test each one.
	std::vector<int> const& lines = LineCache::get_lines(start.xy(), end.xy());
	for (int line_id : lines)
	{
		if (has_los_on_line(start, end, line_id, range))
		{
			return line_id;
		}
	}

	return c_Invalid;
}

bool World::has_los_on_line(Vec3 start, Vec3 end, int line_id, int range) const
{
	PerfTimer perf0("has_los_on_line");

	int cloud_loss = 0;

	LineCache::Itr itr(start.xy(), line_id);
	itr.advance();                  // skip start point
	while (itr && *itr != end.xy()) // skip end point
	{
		if (!permits_sight({itr->x, itr->y, start.z}))
		{
			return false;
		}

		if (range != -1)
		{
			Cloud::Type const cloud = get_cloud(itr->xyz(start.z));
			cloud_loss += Cloud::vision_loss(cloud);
		}

		itr.advance();
	}

	if (range != -1 &&
		!within_range(start, end, range - cloud_loss))
	{
		// The clouds were too thick.
		return false;
	}

	return true;
}

bool World::has_los(Vec3 start, Vec3 end, int range) const
{
	return (get_los(start, end, range) != c_Invalid);
}

void World::draw(Draw::View view) const
{
	PerfTimer perf0("world draw");

	for (Vec2 const& pos : view.view_area())
	{
		draw_map_tile(pos.xyz(view.z), view);
	}

	for (Vec3 tile : view.peek_tiles)
	{
		draw_map_tile(tile, view);
	}
}

void World::draw_map_tile(Vec3 pos, Draw::View const& view) const
{
	Visibility v = get_visibility(pos);
	bool const drawable = (view.ignore_visibility || v == Visibility::Visible || v == Visibility::Explored);
	if (drawable)
	{
		Terrain::Type t = get_terrain(pos);

		// stairs hack - show other stairs when peeking to prevent +- confusion
		if (t == Terrain::UpStairs && Util::Contains(view.peek_tiles, pos))
		{
			t = Terrain::DownStairs;
		}
		else if (t == Terrain::DownStairs && Util::Contains(view.peek_tiles, pos))
		{
			t = Terrain::UpStairs;
		}

		int code = Terrain::get_character(t);
		std::string draw_colour = (v == Visibility::Visible) ? cstr_White : cstr_DarkGrey;

		if (v == Visibility::Visible)
		{
			// Clouds conceal terrain/items at position
			Cloud::Type const cloud = get_cloud(pos);
			if (cloud != Cloud::None)
			{
				code = Cloud::get_codepoint(cloud);
				draw_colour = Cloud::get_colour(cloud);
			}
			else
			{
				// If no cloud, check for item to draw.
				Item::Handle const item = peek_item(pos);
				if (item != c_Invalid)
				{
					code = item.codepoint();
					draw_colour = item.colour();
				}
			}
		}

		const bool highlight_target = Target::is_target(pos) && v == Visibility::Visible;
		if (highlight_target)
		{
			Draw::draw_tile_bg(code, pos.xy(), view, draw_colour.c_str(), Target::colour());
		}
		else
		{
			Draw::draw_tile(code, pos.xy(), view, draw_colour.c_str());
		}
	}
}
