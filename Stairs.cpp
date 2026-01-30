#include "Stairs.h"
#include "Terrain.h"

#include <cassert>

namespace Stairs
{

bool is_up(Direction dir)
{
	return dir >= UpWest
		&& dir <= UpNorth;
}

Vec3 relative_move(Direction dir)
{
	switch (dir)
	{
		case DownEast:  return { 1, 0,-1};
		case DownNorth: return { 0,-1,-1};
		case DownWest:  return {-1, 0,-1};
		case DownSouth: return { 0, 1,-1};
		case UpEast:    return { 1, 0, 1};
		case UpNorth:   return { 0,-1, 1};
		case UpWest:    return {-1, 0, 1};
		case UpSouth:   return { 0, 1, 1};

		default:
			assert(false);
			return{0,0,0};
	}
}

Vec2 joining_vector(Direction dir)
{
	return -1 * relative_move(dir).xy();
}

Terrain::Type get_terrain(Direction dir)
{
	return is_up(dir) ? Terrain::UpStairs : Terrain::DownStairs;
}

Direction corresponding_direction(Direction dir)
{
	switch(dir)
	{
		case DownEast:	return UpWest;
		case DownNorth:	return UpSouth;
		case DownWest:	return UpEast;
		case DownSouth:	return UpNorth;
		case UpEast:	return DownWest;
		case UpNorth:	return DownSouth;
		case UpWest:	return DownEast;
		case UpSouth:	return DownNorth;

		default:
			assert(false);
			return None;
	}
}

Box2 get_box(Vec2 start_pos, Direction dir)
{
	return Box2::spanning(start_pos, start_pos + Stairs::relative_move(dir).xy());
}

} // namespace Stairs
