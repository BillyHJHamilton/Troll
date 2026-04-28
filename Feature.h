#pragma once

#include "Types.h"

#include "Geometry.h"

// Features are interactive terrain tiles that need extra data to function.
// For example, a button needs data about what it triggers.
// This extra data is managed by the Feature module and can be looked up by position.
// Otherwise, Features act like normal terrain.

namespace Feature
{
	void init();
	void clear();
	void serialize(ISerializer& s);

	// Obtains an unused trigger id.
	int get_new_trigger();

	// Adds a feature to the map.
	void spawn(Vec3 pos, Terrain::Type type);

	// Adds a feature that interacts with a trigger.
	void spawn(Vec3 pos, Terrain::Type type, int trigger);

	// Update features that need it.  Runs each turn after the game begins.
	void update_all();

	// Move feature, leaving behind open terrain and stomping terrain at new position.
	void move(Vec3 old_pos, Vec3 new_pos);

	// Damage a feature (if any) at a position:
	void damage(Vec3 pos, Damage::Packet const& damage_packet);

	// Feature-specific functions:
	void open_chest(Vec3 pos);
	void open_portrait(Vec3 pos);
	void unlock_door(Vec3 pos);
	void lock_door(Vec3 pos);
	void activate_flipendo_button(Vec3 pos);
}

