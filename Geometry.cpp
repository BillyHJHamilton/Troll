#include "Geometry.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include <iostream>

inline Vec2 componentwise_min(Vec2 a, Vec2 b)
{
	return {std::min(a.x,b.x), std::min(a.y, b.y)};
}
inline Vec2 componentwise_max(Vec2 a, Vec2 b)
{
	return {std::max(a.x,b.x), std::max(a.y, b.y)};
}

bool check_within_range(Vec2 p0, Vec2 p1, int max_range)
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
	double const squared_distance = (float)(dx * dx + dy * dy);
	return (float)(sqrt(squared_distance));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Boxen

bool Box::contains (const Vec2 &v) const
{
	return v.x < max().x
		&& v.y < max().y
		&& v.x >= min.x
		&& v.y >= min.y;
}

// Use >= and <= because max is +1 past the last occupied cell.
bool Box::intersects (const Box &other) const
{
	if (other.min.x >= max().x
		|| other.min.y >= max().y
		|| other.max().x <= min.x
		|| other.max().y <= min.y)
		return false;
	else
		return true;
}

bool Box::contains (const Box &other) const
{
	for (int i = 0; i < 2; i++)
		if (other.min[i] < min[i] || other.max(i) > max(i))
			return false;
	return true;
}

Box Box::intersection (Box const & other) const
{
	assert(intersects(other));
	Vec2 new_min = componentwise_max(min, other.min);
	Vec2 new_max = componentwise_min(max(), other.max());
	return {new_min, new_max - new_min};
}

//-------------------------------------------------------------------------------------------------
// Rectangle Traversal

BoxItr::BoxItr (Box const & box)
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

BoxItr begin(Box const & b)
{
	return BoxItr(b);
}

BoxItr end(Box const & box)
{
	BoxItr end_itr(box);
	end_itr.current.y = box.max(AXIS_Y);
	return end_itr;
}
