#pragma once

#include "Geometry.h"
#include "Math.h"

// Float geometry
// In a separate header because it's rarely needed in this grid-based game.

//------------------------------------------------------------------------------
// Float Vec2 class

struct FloatVec2
{
	union
	{
		struct
		{
			float x;
			float y;
		};

		float data[2];
	};

	float &operator[] (unsigned int n) { return data[n]; }
	const float &operator[] (unsigned int n) const { return data[n]; }

	static FloatVec2 from_int(Vec2 v) { return {(float)v.x,(float)v.y}; }
	static FloatVec2 from_int(int x, int y) { return {(float)x,(float)y}; }
	Vec2 round_to_int() const { return { Math::RoundToInt(x), Math::RoundToInt(y) }; }
};

inline bool operator== (FloatVec2 lhs, FloatVec2 rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!= (FloatVec2 lhs, FloatVec2 rhs)
{
	return !(operator==(lhs, rhs));
}

inline FloatVec2 operator+ (FloatVec2 lhs, FloatVec2 rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline FloatVec2 operator- (FloatVec2 lhs, FloatVec2 rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline FloatVec2 operator* (float lhs, FloatVec2 rhs)
{
	return {lhs * rhs.x, lhs * rhs.y};
}

inline FloatVec2 operator* (FloatVec2 lhs, float rhs)
{
	return {lhs.x * rhs, lhs.y * rhs};
}

inline FloatVec2 operator/ (FloatVec2 lhs, float rhs)
{
	return {lhs.x / rhs, lhs.y / rhs};
}

inline FloatVec2 const & operator+= (FloatVec2 & lhs, FloatVec2 rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline FloatVec2 const & operator-= (FloatVec2 & lhs, FloatVec2 rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

inline FloatVec2 const & operator*= (FloatVec2 & lhs, float rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	return lhs;
}

inline FloatVec2 const & operator/= (FloatVec2 & lhs, float rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

//------------------------------------------------------------------------------
// Float-based line iterator that converts back to int.
// Each step, it moves along the line by 1 unit in the longer dimension,
// then evaluates the position in the other dimension.

class FloatLineItr
{
public:
	FloatLineItr(FloatVec2 start, FloatVec2 end);

	FloatVec2 current;
	int steps_left;

	void advance();

	// Have we gone past the end point of the line?
	// Note: It is safe to keep advancing and using the line past the end point.
	//   It will continue along the same trajectory.
	bool finished() const { return steps_left < 0; }

	// iterator-style functions
	explicit operator bool() { return !finished(); }
	Vec2 operator*() const { return current.round_to_int(); }
	Vec2 operator++() { advance(); return current.round_to_int(); }
	// post-increment not provided to avoid accidental copy

private:
	Axis long_axis;
	float long_sign;
	float slope;
};
