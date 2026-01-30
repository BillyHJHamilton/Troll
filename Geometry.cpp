#include "Geometry.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include <iostream>

//------------------------------------------------------------------------------
// Vec2

inline Vec2 componentwise_min(Vec2 a, Vec2 b)
{
	return {std::min(a.x,b.x), std::min(a.y, b.y)};
}
inline Vec2 componentwise_max(Vec2 a, Vec2 b)
{
	return {std::max(a.x,b.x), std::max(a.y, b.y)};
}

bool within_range(Vec2 p0, Vec2 p1, int max_range)
{
	assert(max_range < sqrt(INT_MAX));
	int const squared_max_range = max_range * max_range;

	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;
	int const squared_distance = dx*dx + dy*dy;

	return squared_distance <= squared_max_range;
}

float euclidean_distance(Vec2 p0, Vec2 p1)
{
	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;
	double const squared_distance = (float)(dx*dx + dy*dy);
	return (float)(sqrt(squared_distance));
}

//------------------------------------------------------------------------------
// Vec3

inline Vec3 componentwise_min(Vec3 a, Vec3 b)
{
	return { std::min(a.x,b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}
inline Vec3 componentwise_max(Vec3 a, Vec3 b)
{
	return { std::max(a.x,b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

bool within_range(Vec3 p0, Vec3 p1, int max_range)
{
	assert(max_range < sqrt(INT_MAX));
	int const squared_max_range = max_range * max_range;

	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;
	int const dz = p0.z - p1.z;
	int const squared_distance = dx*dx + dy*dy + dz*dz;

	return squared_distance <= squared_max_range;
}

float euclidean_distance(Vec3 p0, Vec3 p1)
{
	int const dx = p0.x - p1.x;
	int const dy = p0.y - p1.y;
	int const dz = p0.z - p1.z;
	int const squared_distance = dx*dx + dy*dy + dz*dz;

	return (float)(sqrt(squared_distance));
}

//------------------------------------------------------------------------------
// Boxen

bool Box2::contains (const Vec2 &v) const
{
	return v.x < max().x
		&& v.y < max().y
		&& v.x >= min.x
		&& v.y >= min.y;
}

// Use >= and <= because max is +1 past the last occupied cell.
bool Box2::intersects (const Box2 &other) const
{
	if (other.min.x >= max().x
		|| other.min.y >= max().y
		|| other.max().x <= min.x
		|| other.max().y <= min.y)
		return false;
	else
		return true;
}

bool Box2::intersects_or_adjacent (const Box2 &other) const
{
	// To check for adjacency, check for intersection with a slightly larger box.
	Box2 temp = Box2(min.x - 1, min.y - 1, size.x + 2, size.y + 2);
	return temp.intersects(other);
}

bool Box2::contains (const Box2 &other) const
{
	if (other.min.x < min.x
		|| other.min.y < min.y
		|| other.max().x > max().x
		|| other.max().y > max().y)
		return false;
	else
		return true;
}

bool Box2::overlaps_on_axis(Box2 const & other, Axis axis) const
{
	return other.max()[axis] > min[axis]
		&& other.min[axis] < max()[axis];
}

Box2 Box2::intersection (Box2 const & other) const
{
	assert(intersects(other));
	Vec2 new_min = componentwise_max(min, other.min);
	Vec2 new_max = componentwise_min(max(), other.max());
	return {new_min, new_max - new_min};
}

//------------------------------------------------------------------------------
// 3D Boxen

bool Box3::contains(const Vec3& v) const
{
	return v.x < max().x
		&& v.y < max().y
		&& v.z < max().z
		&& v.x >= min.x
		&& v.y >= min.y
		&& v.z >= min.z;
}

// Use >= and <= because max is +1 past the last occupied cell.
bool Box3::intersects(const Box3& other) const
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

bool Box3::contains(const Box3& other) const
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

Box3 Box3::intersection(Box3 const& other) const
{
	assert(intersects(other));
	Vec3 new_min = componentwise_max(min, other.min);
	Vec3 new_max = componentwise_min(max(), other.max());
	return { new_min, new_max - new_min };
}

//-------------------------------------------------------------------------------------------------
// Rectangle Traversal

BoxItr::BoxItr (Box2 const & box)
	: box(box)
	, current(box.min)
{ }

void BoxItr::advance ()
{
	++ current.x;
	if (current.x >= box.max(AXIS_X))
	{
		current.x = box.min.x;
		++ current.y;
	}
}

bool BoxItr::finished () const
{
	return current.y >= box.max(AXIS_Y);
}

BoxItr begin(Box2 const & b)
{
	return BoxItr(b);
}

BoxItr end(Box2 const & box)
{
	BoxItr end_itr(box);
	end_itr.current.y = box.max(AXIS_Y);
	return end_itr;
}
