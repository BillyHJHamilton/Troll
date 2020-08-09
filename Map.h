#pragma once

#include "Types.h"
#include "Geometry.h"

#include <vector>

//int constexpr MAP_WIDTH = 100;
//int constexpr MAP_HEIGHT = 100;

struct DrawView;

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

struct Map
{
	// data
	Box map_box;
private:
	Grid<Terrain> terrain;		 // hidden because local coords are confusing
	Grid<Visibility> visibility; // hidden because local coords are confusing

	 // functions
public:
	void init(Box const & box, Terrain fill);

	Terrain get_terrain(Vec2 const & global_pos) const;
	Visibility get_visibility(Vec2 const & global_pos) const;
	void set_terrain(Vec2 const & global_pos, Terrain t);
	void set_visibility(Vec2 const & global_pos, Visibility v);

	bool tile_is_solid(Vec2 const & global_pos) const;

	void fill(Terrain t);
	void fill_box(Box const & r, Terrain t);

	void clear_visibility();
	void update_visibility(Vec2 const & viewer, int max_radius);

	void draw_map_tile (Vec2 global_pos, DrawView const & view, bool ignore_visibility);
	void draw(DrawView const & view, bool ignore_visibility=false);

	inline Vec2 global_to_local(Vec2 const & global) const { return global - map_box.min; }
	inline bool local_pos_valid(Vec2 const & local_pos) const { return Box{Vec2{0,0}, map_box.size}.contains(local_pos); }
	inline bool contains(Box const & box) const { return map_box.contains(box); }
	inline bool contains(Vec2 const & global_pos) const { return map_box.contains(global_pos); }
};

bool check_los(Map const & map, Vec2 const & p0, Vec2 const & p1);
