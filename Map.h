#pragma once

#include "Types.h"
#include "Item.h"
#include "Cloud.h"
#include "Geometry.h"
#include "Stairs.h"

#include <memory>
#include <unordered_map>
#include <vector>

enum class Visibility : byte
{
	Hidden = 0,
	Explored,
	Visible
};

class Map
{
	// functions
public:
	void init(int z, float map_difficulty, Box2 box, Terrain::Type fill);

	MapGenerator& get_generator();

	Box2 get_box() const { return map_box; }
	int get_z() const { return global_z; }
	float get_difficulty() const { return difficulty; }

	inline Vec2 global_to_local(Vec2 global) const { return global - map_box.min; }
	inline bool local_pos_valid(Vec2 local_pos) const { return Box2{Vec2{0,0}, map_box.size}.contains(local_pos); }
	inline bool contains(Vec2 global_pos) const { return map_box.contains(global_pos); }
	inline bool contains(Vec3 global_pos) const { return global_pos.z == global_z && map_box.contains({global_pos.x, global_pos.y}); }
	inline bool contains(Box2 box) const { return map_box.contains(box); }

	Terrain::Type get_terrain(Vec2 global_pos) const;
	Visibility get_visibility(Vec2 global_pos, int current_step) const;
	Cloud::Type get_cloud(Vec2 global_pos) const;
	int get_cloud_lifetime(Vec2 global_pos) const;

	void set_terrain(Vec2 global_pos, Terrain::Type t);
	void set_visibility(Vec2 global_pos, Visibility v, int current_step);
	void set_all_explored();

	bool try_add_cloud(Vec2 global_pos, Cloud::Type cloud, int lifetime);
	void clear_cloud(Vec2 global_pos);
	void step_clouds();
	void clear_clouds();

	bool has_item(Vec2 global_pos) const;
	Item::Handle const peek_item(Vec2 global_pos) const;
	//std::vector<Item::Handle> get_items(Vec2 global_pos) const;
	void add_item(Vec2 global_pos, Item::Handle item);
	Item::Handle pop_item(Vec2 global_pos);

	bool tile_is_solid(Vec2 global_pos) const;
	bool tile_permits_sight(Vec2 global_pos) const;

	void fill(Terrain::Type t);
	void fill_box(Box2 global_box, Terrain::Type t);

	void add_stairs(Vec2 global_pos, Stairs::Direction dir);
	Stairs::Direction get_stairs(Vec2 global_pos) const;
	bool has_stairs(Vec2 global_pos) const;
	std::unordered_map<Vec2,Stairs::Direction> const& get_stairs_map() const { return stairs; }

	void add_corresponding_stairs(const Map& other);
	void remove_stairs(Vec2 global_pos);

	void clear_visibility(int current_step);
	void clean_explored_values(int current_step);

protected:
	// Area occupied by this map in global space.
	Box2 map_box = {};

	// All maps are flat, so it only needs a single z coordinate.
	int global_z = 0;

	// Map's difficulty rating, for creature and item placement.
	float difficulty = 0.0f;

	// Data in local space.  Warning: Local coords may be confusing.
	// Use the get/set functions if you want to do things in global space.
	Grid<Terrain::Type> terrain;
	Grid<int> visibility;
	Grid<Cloud::Type> clouds;
	Grid<Item::Handle> items;

	// Lifetimes of clouds currently active on the map.  Key is global pos.
	std::unordered_map<Vec2,int> cloud_lifetimes;

	// List of stairs on the level.  Key is global pos for local end of the stairs.
	std::unordered_map<Vec2,Stairs::Direction> stairs;

	std::shared_ptr<MapGenerator> generator;
};
