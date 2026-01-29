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

	struct Data
	{
		Vec2 start_pos;
		Direction direction;
	};

	bool is_up(Direction dir);
	Vec3 relative_move(Direction dir);
	Terrain::Type get_terrain(Direction dir);
	Direction corresponding_direction(Direction dir);
}