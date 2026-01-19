#pragma once

#pragma once

#include <cmath>
#include "Geometry.h"

namespace Math
{
	static constexpr float c_FloatEpsilon = 0.001f;

	static constexpr double c_Pi = 3.14159265358979323846;
	static constexpr double c_TwoPi = 2.0 * c_Pi;

	inline int Sign(int x)
	{
		return (x > 0) ? 1 : (x == 0) ? 0 : -1;
	}

	inline float FloatSign(int x)
	{
		return (x > 0) ? 1.0f : (x == 0) ? 0.0f : -1.0f;
	}

	inline float Sign(float x)
	{
		return (x > c_FloatEpsilon) ? 1.0f : (x < -c_FloatEpsilon) ? -1.0f : 0.0f;
	}

	inline int IntSign(float x)
	{
		return (x > c_FloatEpsilon) ? 1 : (x < -c_FloatEpsilon) ? -1 : 0;
	}

	inline bool FloatIsZero(float x)
	{
		return abs(x) < c_FloatEpsilon;
	}

	inline bool FloatNearlyZero(float x, float tolerance)
	{
		return abs(x) < tolerance;
	}

	inline bool FloatEquals(float a, float b)
	{
		return abs(a - b) < c_FloatEpsilon;
	}

	inline bool FloatNearlyEquals(float a, float b, float tolerance)
	{
		return abs(a - b) < tolerance;
	}

	inline bool FloatLess(float a, float b)
	{
		return a < b - c_FloatEpsilon;
	}

	inline bool FloatGreater(float a, float b)
	{
		return a > b + c_FloatEpsilon;
	}

	inline bool FloatLessOrEqual(float a, float b)
	{
		return a < b + c_FloatEpsilon;
	}

	inline bool FloatGreaterOrEqual(float a, float b)
	{
		return a > b - c_FloatEpsilon;
	}

	template<typename T>
	inline T Clamp(T value, T low, T high)
	{
		return (value < low) ? low : (value < high) ? value : high;
	}

/*	inline float LengthSquared(sf::Vector2f v)
	{
		return v.x * v.x + v.y * v.y;
	}
	inline float Length(sf::Vector2f v)
	{
		return std::sqrt(LengthSquared(v));
	}
	inline float DistanceSquared(sf::Vector2f p1, sf::Vector2f p2)
	{
		return LengthSquared(p2 - p1);
	}
	inline float Distance(sf::Vector2f p1, sf::Vector2f p2)
	{
		return Length(p2 - p1);
	}*/

	inline float DegToRad(float degrees)
	{
		return 0.01745329252f * degrees;
	}
	inline float SinDeg(float degrees)
	{
		return std::sinf(DegToRad(degrees));
	}
	inline float CosDeg(float degrees)
	{
		return std::cosf(DegToRad(degrees));
	}

/*	inline sf::Vector2f RotationToUnitVector(float sfRotation)
	{
		return
		{
			Math::SinDeg(sfRotation),
			-Math::CosDeg(sfRotation)
		};
	}*/
};
