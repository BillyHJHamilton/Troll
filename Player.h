#pragma once

#include "Geometry.h"
#include "Types.h"

namespace Player
{
	static int constexpr vision_radius = 8;

	struct Data
	{
		bool acted = false;
		bool game_over = false;
		Creature::Type defeated_by = (Creature::Type)-1;
	};

	void clear();

	Vec3 pos ();
	Data& data ();
	Creature::Handle handle () { return 0; }

	void set_acted(bool acted);
	void set_game_over(Creature::Type instigator);
};
