#pragma once

#include "Types.h"
#include "Map.h"
#include <vector>

// World stores all the maps in the game.
// It is used to resolve functions in global space by referring to the relevant map.
class World
{
public:
	static void clear();
	static World& edit();
	static World const& read();

	int add_map(int z, Box2 box, Terrain fill);
	int num_maps() const { return (int)maps.size(); }
	Map& get_map(int index) { return maps.at(index); }
	const Map& get_map(int index) const { return maps.at(index); }

	Terrain get_terrain(Vec3 pos) const;
	bool is_solid(Vec3 pos) const;
	bool permits_sight(Vec3 pos) const;

	Visibility get_visibility(Vec3 pos) const;
	void update_visibility(Vec3 viewer, int vision_radius);

	// Find index of the first map (hopefully only) containing the given position.
	// If no map contains the position, returns c_invalid.
	int find_map(Vec3 global_pos) const;

	// Returns the id of a clear line (in the line cache) from start to end.
	// If no clear line exists, returns c_invalid.
	int get_los(Vec3 start, Vec3 end) const;

	// Checks los along a single line trajectory.
	// The line_id must be a valid line from start to end.
	// Called by get_los for each relevant trajectory.
	bool has_los_on_line(Vec3 start, Vec3 end, int line_id) const;

	// Runs get_los and returns true if the result was not c_invalid.
	// Use this if you don't plan to use the line you found.
	bool has_los(Vec3 start, Vec3 end) const;

	void draw(Draw::View view, bool ignore_visibility) const;

private:
	std::vector<Map> maps;

	// speed up map searches by first checking the one it was last time
	mutable int temp_last_map = c_invalid;
};
