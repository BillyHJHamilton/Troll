#pragma once

#include "Geometry.h"
#include "Types.h"

namespace Player
{
	static int constexpr vision_radius = 8;

	struct Data
	{
		CompassDirection automove = c_CompassInvalid;
		bool acted = false;

		bool game_over = false;
		Creature::Type defeated_by = (Creature::Type)-1;

		int level = 1;
		int xp = 0;
	};

	// Initialization
	void clear();

	// Getters
	Vec3 pos ();
	Creature::Handle handle ();
	bool is_automoving ();
	CompassDirection get_automove ();
	bool has_acted ();
	bool is_game_over ();
	Creature::Type get_defeated_by ();
	int current_level ();
	int current_xp ();
	int next_xp_threshold ();

	// Mutators
	void start_automove (CompassDirection dir);
	void stop_automove ();
	void set_acted (bool acted);
	void set_game_over (Creature::Type instigator);
	void gain_xp_for (Creature::Type creature);
};
