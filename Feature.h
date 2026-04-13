#pragma once

#include "Types.h"

#include "Geometry.h"

// Features are interactive terrain tiles that need extra data to function.
// For example, a switch needs data about what it triggers.
// This extra data is managed by the Feature module and can be looked up by position.
// Otherwise, Features act like normal terrain.

namespace Feature
{
	void init();
	void clear();
	void serialize(ISerializer& s);

	// Adds a feature to the map.
	void spawn(Vec3 pos, Terrain::Type type);

	// Adds a feature to the map.
	void spawn_flipendo_switch(Vec3 switch_pos, Vec3 door_pos);

	// Move feature, leaving behind open terrain and stomping terrain at new position.
	void move(Vec3 old_pos, Vec3 new_pos);

	// Remove feature and replace with the indicated terrain.
	void remove(Vec3 pos, Terrain::Type new_terrain_type);

	// Remove feature and replace with open terrain.
	void remove(Vec3 pos) { remove(pos, Terrain::Open); }

	// Feature-specific functions:
	void open_chest(Vec3 pos);
	void open_portrait(Vec3 pos);
	void activate_flipendo_switch(Vec3 pos);
}

