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

	// Check if there is a feature at a position
	bool is_at(const Vec3 & pos);

	// Adds a feature to the map.
	void spawn(const Vec3 & pos, Terrain::Type type);

	// Move feature, leaving behind open terrain and stomping terrain at new position.
	void move(const Vec3 & old_pos, const Vec3 & new_pos);

	// Remove feature and replace with open terrain.
	void remove(const Vec3 & pos);

	// Feature-specific functions:
	void open_chest(const Vec3 & pos);
	void open_portrait(const Vec3 & pos);
}

