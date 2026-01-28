#pragma once

#include "Types.h"
#include "Geometry.h"

#include <vector>

//int constexpr MAP_WIDTH = 100;
//int constexpr MAP_HEIGHT = 100;

enum class Terrain : byte
{
	Open = 0,
	Wall
};

enum class Visibility : byte
{
	Hidden = 0,
	Explored,
	Visible
};

int terrain_character(Terrain t);
bool terrain_permits_sight(Terrain t);
bool terrain_is_solid(Terrain t);

class Map
{
	// functions
public:
	void init(int z, Box2 const& box, Terrain fill);

	int get_z() const { return global_z; }

	Terrain get_terrain(Vec2 const & global_pos) const;
	Visibility get_visibility(Vec2 const & global_pos, int current_step) const;
	void set_terrain(Vec2 const & global_pos, Terrain t);
	void set_visibility(Vec2 const & global_pos, Visibility v, int current_step);

	bool tile_is_solid(Vec2 const & global_pos) const;
	bool tile_permits_sight(Vec2 const& global_pos) const;

	void fill(Terrain t);
	void fill_box(Box2 const & global_box, Terrain t);

	void clear_visibility(int current_step);
	void clean_explored_values(int current_step);

	inline Vec2 global_to_local(Vec2 const & global) const { return global - map_box.min; }
	inline bool local_pos_valid(Vec2 const & local_pos) const { return Box2{Vec2{0,0}, map_box.size}.contains(local_pos); }
	inline bool contains(Vec2 const & global_pos) const { return map_box.contains(global_pos); }
	inline bool contains(Vec3 const& global_pos) const { return global_pos.z == global_z && map_box.contains({global_pos.x, global_pos.y}); }
	inline bool contains(Box2 const& box) const { return map_box.contains(box); }

	void test_los_symmetry();

private:
	// Area occupied by this map in global space.
	Box2 map_box = {};

	// All maps are flat, so it only needs a single z coordinate.
	int global_z = 0;

	// Data in local space.  Warning: Local coords may be confusing.
	// Use the get/set functions if you want to do things in global space.
	Grid<Terrain> terrain;
	Grid<int> visibility;
};

// Returns the id of a clear line (in the cache) from p0 to p1.
// If no clear line exists, returns -1.
int get_los(Map const & map, Vec2 const & p0, Vec2 const & p1);

// Checks whether get_los would find any line.
// Use this if you don't plan to use the line you found.
bool has_los(Map const& map, Vec2 const& p0, Vec2 const& p1);
