#include "Geometry.h"

#include "Debug.h"

#include <algorithm>
#include <cassert>
#include <cmath>

//------------------------------------------------------------------------------
// Vec2

CompassDirection to_compass(Vec2 vec)
{
	switch(vec.x)
	{
		case -1:
		{
			switch(vec.y)
			{
				case -1: return c_CompassNorthwest;
				case  0: return c_CompassWest;
				case  1: return c_CompassSouthwest;
			}
			break;
		}

		case 0:
		{
			switch(vec.y)
			{
				case -1: return c_CompassNorth;
				case  0: return c_CompassNoMove;
				case  1: return c_CompassSouth;
			}
			break;
		}

		case 1:
		{
			switch(vec.y)
			{
				case -1: return c_CompassNortheast;
				case  0: return c_CompassEast;
				case  1: return c_CompassSoutheast;
			}
			break;
		}
	}

	return c_CompassInvalid;
}

inline Vec2 componentwise_min(Vec2 a, Vec2 b)
{
	return {std::min(a.x,b.x), std::min(a.y, b.y)};
}
inline Vec2 componentwise_max(Vec2 a, Vec2 b)
{
	return {std::max(a.x,b.x), std::max(a.y, b.y)};
}

int square_dist(Vec2 p0, Vec2 p1)
{
	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;

	// Make sure we aren't near an int overflow.
	assert(abs(dx) < 30'000
		&& abs(dy) < 30'000);

	return dx*dx + dy*dy;
}

bool strict_range(Vec2 p0, Vec2 p1, int max_range)
{
	assert(max_range < sqrt(INT_MAX));
	int const squared_max_range = max_range * max_range;
	return square_dist(p0,p1) <= squared_max_range;
}

bool float_range(Vec2 p0, Vec2 p1, float max_range)
{
	assert(max_range < sqrt(FLT_MAX));
	float const squared_max_range = max_range * max_range;
	return (float)square_dist(p0,p1) <= squared_max_range;
}

float euclid(Vec2 p0, Vec2 p1)
{
	return (float)(sqrt(square_dist(p0,p1)));
}

int manhattan(Vec2 p0, Vec2 p1)
{
	int const dx = abs(p0.x - p1.x);
	int const dy = abs(p0.y - p1.y);
	return dx + dy;
}

int chessboard(Vec2 p0, Vec2 p1)
{
	int const dx = abs(p0.x - p1.x);
	int const dy = abs(p0.y - p1.y);
	return std::max(dx, dy);
}

Axis get_long_axis(Vec2 v)
{
	if (abs(v.x) >= abs(v.y))
	{
		return c_AxisX;
	}
	else
	{
		return c_AxisY;
	}
}

Vec2 truncate_to_unit(Vec2 a)
{
	float constexpr c_Cutoff = 0.4142135623730950488016887242097f;  // tan(22.5)

	if (a.x == 0 && a.y == 0)
		return a;

	// algorithm:
	//  1. truncate vector to +- 1 along each axis
	//    -> I use sign/signum for this
	//  2. set small components to 0
	//    -> small is compared to the other component
	//    -> for angles close to cardinal directions but not quite aligned
	//    -> the cutoff is a 22.5 degree angle, so each direction gets 1/8 of a circle

	// examples:
	//  ( 0,  0)  =>  ( 0,  0)
	//  ( 1,  0)  =>  ( 1,  0)
	//  (-1,  1)  =>  (-1,  1)
	//  ( 0, -8)  =>  ( 0, -1)
	//  ( 4, -2)  =>  ( 1, -1)  // 26.6 degrees, just past cutoff
	//  (-5, -1)  =>  (-1,  0)

	// C++ has no signum function, so I implement it as
	//   (x > 0) - (x < 0)
	Vec2 result = { (a.x > 0) - (a.x < 0),
					(a.y > 0) - (a.y < 0) };

	int const abs_x = abs(a.x);
	int const abs_y = abs(a.y);
	float const larger = (abs_x > abs_y) ? (float)(abs_x) : (float)(abs_y);
	float const frac_x = a.x / larger;
	float const frac_y = a.y / larger;
	if (abs(frac_x) < c_Cutoff)
		result.x = 0;
	if (abs(frac_y) < c_Cutoff)
		result.y = 0;
	return result;
}

//------------------------------------------------------------------------------
// Vec3

Vec3 Vec3::adjusted(Axis a, int n) const
{
	assert(a >= 0 && a <= 2);
	Vec3 copy = *this;
	copy[a] += n;
	return copy;
}

inline Vec3 componentwise_min(Vec3 a, Vec3 b)
{
	return { std::min(a.x,b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}
inline Vec3 componentwise_max(Vec3 a, Vec3 b)
{
	return { std::max(a.x,b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

int square_dist_3d(Vec3 p0, Vec3 p1)
{
	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;
	int const dz = p0.z - p1.z;

	// Make sure we aren't near an int overflow.
	assert(abs(dx) < 20'000
		&& abs(dy) < 20'000
		&& abs(dz) < 20'000);

	return dx*dx + dy*dy + dz*dz;
}

bool strict_range_3d(Vec3 p0, Vec3 p1, int max_range)
{
	assert(max_range < sqrt(INT_MAX));
	int const squared_max_range = max_range * max_range;
	return square_dist_3d(p0,p1) <= squared_max_range;
}

bool range_3d(Vec3 p0, Vec3 p1, float max_range)
{
	assert(max_range < sqrt(FLT_MAX));
	float const squared_max_range = max_range * max_range;
	return (float)square_dist_3d(p0,p1) <= squared_max_range;
}

float euclid_3d(Vec3 p0, Vec3 p1)
{
	return (float)(sqrt(square_dist_3d(p0,p1)));
}

int manhattan_3d(Vec3 p0, Vec3 p1)
{
	int const dx = abs(p0.x - p1.x);
	int const dy = abs(p0.y - p1.y);
	int const dz = abs(p0.z - p1.z);
	return dx + dy + dz;
}

int chessboard_3d(Vec3 p0, Vec3 p1)
{
	int const dx = abs(p0.x - p1.x);
	int const dy = abs(p0.y - p1.y);
	int const dz = abs(p0.z - p1.z);
	return std::min(std::min(dx, dy), dz);
}

//------------------------------------------------------------------------------
// Intervals

bool Interval::overlaps(Interval other) const
{
	return other.max > min && other.min < max;
}

Interval Interval::overlap(Interval other) const
{
	return {std::max(min, other.min), std::min(max, other.max)};
}

/*static*/ Interval Interval::spanning(int a, int b)
{
	return Interval{std::min(a,b), std::max(a,b)};
}

//------------------------------------------------------------------------------
// Boxen

bool Box2::contains (Vec2 v) const
{
	return v.x < max().x
		&& v.y < max().y
		&& v.x >= min.x
		&& v.y >= min.y;
}

bool Box2::contains (Box2 other) const
{
	if (other.min.x < min.x
		|| other.min.y < min.y
		|| other.max().x > max().x
		|| other.max().y > max().y)
		return false;
	else
		return true;
}

// Use >= and <= because max is +1 past the last occupied cell.
bool Box2::intersects (Box2 other) const
{
	if (other.min.x >= max().x
		|| other.min.y >= max().y
		|| other.max().x <= min.x
		|| other.max().y <= min.y)
		return false;
	else
		return true;
}

bool Box2::intersects_or_adjacent (Box2 other) const
{
	// To check for adjacency, check for intersection with a slightly larger box.
	Box2 temp = Box2(min.x - 1, min.y - 1, size.x + 2, size.y + 2);
	return temp.intersects(other);
}

Box2 Box2::intersection (Box2 other) const
{
	assert(intersects(other));
	Vec2 new_min = componentwise_max(min, other.min);
	Vec2 new_max = componentwise_min(max(), other.max());
	return {new_min, new_max - new_min};
}

CompassDirection Box2::adjacent_edge (Box2 other) const
{
	if (other.min.x == max(c_AxisX) && overlaps_on_axis(other, c_AxisY))
	{
		return c_CompassEast;
	}
	else if (other.max(c_AxisX) == min.x && overlaps_on_axis(other, c_AxisY))
	{
		return c_CompassWest;
	}
	else if (other.max(c_AxisY) == min.y && overlaps_on_axis(other, c_AxisX))
	{
		return c_CompassNorth;
	}
	else if (other.min.y == max(c_AxisY) && overlaps_on_axis(other, c_AxisX))
	{
		return c_CompassSouth;
	}
	else
	{
		return c_CompassInvalid;
	}
}

Interval Box2::interval_on_axis (Axis a) const
{
	return {min[a], max(a)};
}

bool Box2::overlaps_on_axis(Box2 other, Axis a) const
{
	return interval_on_axis(a).overlaps(other.interval_on_axis(a));
}

Interval Box2::overlap_on_axis(Box2 other, Axis a) const
{
	return interval_on_axis(a).overlap(other.interval_on_axis(a));
}

Box2 Box2::minus_border(int border_size) const
{
	assert(border_size*2 < size.x && border_size*2 < size.y);
	return Box2{
		min + Vec2{border_size, border_size},
		size - 2*Vec2{border_size, border_size}
	};
}

Box2 Box2::plus_border(int border_size) const
{
	return Box2{
		min - Vec2{border_size, border_size},
		size + 2*Vec2{border_size, border_size}
	};
}

Vec2 Box2::snap_to_inner_border(Vec2 v, CompassDirection edge) const
{
	switch(edge)
	{
		case c_CompassEast:
			return Vec2{inner_max(c_AxisX), v.y};
		case c_CompassNorth:
			return Vec2{v.x, min.y};
		case c_CompassWest:
			return Vec2{min.x, v.y};
		case c_CompassSouth:
			return Vec2{v.x, inner_max(c_AxisY)};
		default:
			DebugBreak();
			return v;
	}
}

Vec2 Box2::snap_to_outer_border(Vec2 v, CompassDirection edge) const
{
	switch(edge)
	{
		case c_CompassEast:
			return Vec2{max(c_AxisX), v.y};
		case c_CompassNorth:
			return Vec2{v.x, min.y - 1};
		case c_CompassWest:
			return Vec2{min.x - 1, v.y};
		case c_CompassSouth:
			return Vec2{v.x, max(c_AxisY)};
		default:
			DebugBreak();
			return v;
	}
}

Box2 Box2::inner_border_box(CompassDirection edge) const
{
	switch(edge)
	{
		case c_CompassEast:
			return Box2{inner_max(c_AxisX), min.y, 0, size.y};
		case c_CompassNorth:
			return Box2{min.x, min.y, size.x, 1};
		case c_CompassWest:
			return Box2{min.x, min.y, 1, size.y};
		case c_CompassSouth:
			return Box2{min.x, inner_max(c_AxisY), size.x, 1};
		default:
			DebugBreak();
			return *this;
	}
}

Box2 Box2::outer_border_box(CompassDirection edge) const
{
	switch(edge)
	{
		case c_CompassEast:
			return Box2{max(c_AxisX), min.y, 1, size.y};
		case c_CompassNorth:
			return Box2{min.x, min.y - 1, size.x, 1};
		case c_CompassWest:
			return Box2{min.x - 1, min.y, 1, size.y};
		case c_CompassSouth:
			return Box2{min.x, max(c_AxisY), size.x, 1};
		default:
			DebugBreak();
			return *this;
	}
}

//------------------------------------------------------------------------------
// 3D Boxen

bool Box3::contains(Vec3 v) const
{
	return v.x < max().x
		&& v.y < max().y
		&& v.z < max().z
		&& v.x >= min.x
		&& v.y >= min.y
		&& v.z >= min.z;
}

// Use >= and <= because max is +1 past the last occupied cell.
bool Box3::intersects(Box3 other) const
{
	if (other.min.x >= max().x
		|| other.min.y >= max().y
		|| other.min.z >= max().z
		|| other.max().x <= min.x
		|| other.max().y <= min.y
		|| other.max().z <= min.z)
		return false;
	else
		return true;
}

bool Box3::contains(Box3 other) const
{
	if (other.min.x < min.x
		|| other.min.y < min.y
		|| other.min.z < min.z
		|| other.max().x > max().x
		|| other.max().y > max().y
		|| other.max().z > max().z)
		return false;
	else
		return true;

}

Box3 Box3::intersection(Box3 other) const
{
	assert(intersects(other));
	Vec3 new_min = componentwise_max(min, other.min);
	Vec3 new_max = componentwise_min(max(), other.max());
	return { new_min, new_max - new_min };
}

//-------------------------------------------------------------------------------------------------
// Rectangle Traversal

BoxItr::BoxItr (Box2 box)
	: box(box)
	, current(box.min)
{ }

void BoxItr::advance ()
{
	++ current.x;
	if (current.x >= box.max(c_AxisX))
	{
		current.x = box.min.x;
		++ current.y;
	}
}

bool BoxItr::finished () const
{
	return current.y >= box.max(c_AxisY);
}

BoxItr begin(Box2 b)
{
	return BoxItr(b);
}

BoxItr end(Box2 box)
{
	BoxItr end_itr(box);
	end_itr.current.y = box.max(c_AxisY);
	return end_itr;
}
