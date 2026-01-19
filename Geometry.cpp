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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Line Drawing

LineItr::LineItr (Vec2 const & start, Vec2 const & end)
	: current(start)
{
	// based on https://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm

	int const dx = end.x - start.x;
	int const dy = end.y - start.y;

	int const dist_x = std::abs(dx);
	int const dist_y = std::abs(dy);

	int const step_x = (dx > 0) ? 1 : -1;
	int const step_y = (dy > 0) ? 1 : -1;

	int dist_short;

	if (dist_x > dist_y)
	{
		dist_long = dist_x;
		dist_short = dist_y;

		step[0] = {step_x, 0};		// step on one axis
		step[1] = {step_x, step_y}; // step on both axes
	}
	else
	{
		dist_long = dist_y;
		dist_short = dist_x;

		step[0] = {0, step_y};		// step on one axis
		step[1] = {step_x, step_y}; // step on both axes
	}

	d_error[0] = {dist_short};				// change in error when taking step on one axis
	d_error[1] = {dist_short - dist_long};	// change in error when taking step on both axes

	steps_left = dist_long;
	error = dist_short;
}

void LineItr::advance()
{
	// based on https://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm

	int const error_too_big = error >= dist_long;
	// dist_long / 2 makes nicer line, but does strange things to visibility

	current += step[error_too_big];
	error += d_error[error_too_big];

	-- steps_left;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
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
