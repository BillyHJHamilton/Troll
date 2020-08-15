#pragma once

#include "Geometry.h"
#include "Types.h"

struct Map;
struct DrawView;

struct Player
{
	int vision_radius = 8;

	static Vec2 const & pos ();

	static bool try_move (Vec2 const & relative_move);

	static Creature::Handle handle () { return 0; }
};

Player & g_player();
