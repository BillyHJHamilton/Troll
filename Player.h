#pragma once

#include "Geometry.h"
#include "Types.h"

struct Map;
struct DrawView;

struct Player
{
	//--------------------------------------------------------------------------
	// Player-specific data

	static int constexpr vision_radius = 8;

	bool acted = false;

	//--------------------------------------------------------------------------
	// Static interface

	static void clear();
	static Vec2 const & pos ();
	static Creature::Handle handle () { return 0; }
};

Player & g_player();
