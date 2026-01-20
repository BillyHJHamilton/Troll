#pragma once

#include "Geometry.h"
#include "Types.h"

struct Player
{
	//--------------------------------------------------------------------------
	// Player-specific data

	static int constexpr vision_radius = 8;

	bool acted = false;
	bool game_over = false;
	Creature::Type defeated_by = (Creature::Type)-1;

	//--------------------------------------------------------------------------
	// Static interface

	static void clear();
	static Vec2 const & pos ();
	static Creature::Handle handle () { return 0; }
};

Player & g_player();
