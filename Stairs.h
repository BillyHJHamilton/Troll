#pragma once

#include "Geometry.h"
#include "Types.h"

namespace Stairs
{
	enum Direction : byte
	{
		None,
		DownEast, DownNorth, DownWest, DownSouth,
		UpWest,   UpSouth,   UpEast,   UpNorth
	};

	// Type used by stairs hashmap.
	using Pair = std::pair<Vec2,Stairs::Direction>;

	bool is_up(Direction dir);
	Vec3 relative_move(Direction dir);
	Vec2 joining_vector(Direction dir);
	Terrain::Type get_terrain(Direction dir);
	Direction reverse(Direction dir);
	Box2 get_box(Vec2 start_pos, Direction dir);

	Stairs::Direction random_up_direction();
	Stairs::Direction random_down_direction();
}