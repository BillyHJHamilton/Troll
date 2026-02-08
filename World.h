#pragma once

#include "Geometry.h"
#include "Types.h"

#include <memory>
#include <vector>

// World stores all the maps in the game.
// It is used to resolve functions in global space by referring to the relevant map.
class World
{
public:
	static void clear();
	static World& edit();
	static World const& read();

	int add_map(int z, Box2 box, Terrain::Type fill);
	int num_maps() const;
	Map& edit_map(int index);
	const Map& read_map(int index) const;

	// Find index of the first map (hopefully only) containing the given position.
	// If no map contains the position, returns c_invalid.
	int find_map(Vec3 global_pos) const;

	Terrain::Type get_terrain(Vec3 pos) const;
	bool is_solid(Vec3 pos) const;
	bool permits_sight(Vec3 pos) const;

	// Clouds
	Cloud::Type get_cloud(Vec3 pos) const;
	int get_cloud_lifetime(Vec3 pos) const;
	bool try_add_cloud(Vec3 pos, Cloud::Type cloud, int lifetime);
	void clear_cloud(Vec3 pos);
	void step_clouds();
	void clear_clouds();

	// Returns direction of stairs starting at pos, or Stairs::None.
	Stairs::Direction get_stairs(Vec3 pos) const;

	// Calls get_stairs and returns true if result was not Stairs::None.
	bool has_stairs(Vec3 pos) const;

	// Returns z change from stairs when trying to move from old_pos to new_pos.
	// Returns 0 if there are no stairs starting at old_pos.
	int get_stairs_dz(Vec3 old_pos, Vec2 new_pos) const;

	Visibility get_visibility(Vec3 pos) const;
	bool is_visible(Vec3 pos) const;
	void set_visibility(Vec3 pos, Visibility v);
	void update_visibility(Vec3 viewer, int vision_radius);

	// Returns the id of a clear line (in the line cache) from start to end.
	// If no clear line exists, returns c_invalid.
	int get_los(Vec3 start, Vec3 end, int range) const;

	// Checks los along a single line trajectory.
	// The line_id must be a valid line from start to end.
	// Called by get_los for each relevant trajectory.
	bool has_los_on_line(Vec3 start, Vec3 end, int line_id, int range) const;

	// Runs get_los and returns true if the result was not c_invalid.
	// Use this if you don't plan to use the line you found.
	bool has_los(Vec3 start, Vec3 end, int range) const;

	void draw(Draw::View view) const;

private:
	void add_stairs_visibility(Vec3 viewer);
	void wall_visibility_hack(Vec3 viewer, Axis a, int sign);
	void advance_visibility_step();
	void draw_map_tile(Vec3 pos, Draw::View const& view) const;

	std::vector<std::shared_ptr<Map>> maps;

	// Anything with this number in the vis map is visible.
	// Anything with another non-negative number is remembered.
	// Anything with negative number is unseen.
	int visibility_step = 1;

	// speed up map searches by first checking the one it was last time
	mutable int temp_last_map = c_invalid;
};
