#include "FloatGeometry.h"

#include "Debug.h"

FloatLineItr::FloatLineItr(FloatVec2 start, FloatVec2 end)
{
	current = start;
	if (!Check(end != start))
	{
		steps_left = 0;
		long_axis = AXIS_X;
		long_sign = 0;
		slope = 0.0f;
		return;
	}

	float dx = end.x - start.x;
	float dy = end.y - start.y;

	if (abs(dx) > abs(dy))
	{
		steps_left = Math::RoundToInt(abs(dx));
		long_axis = AXIS_X;
		long_sign = Math::Sign(dx);
		slope = dy/abs(dx);
	}
	else
	{
		steps_left = Math::RoundToInt(abs(dy));
		long_axis = AXIS_Y;
		long_sign = Math::Sign(dy);
		slope = dx/abs(dy);
	}
}

void FloatLineItr::advance()
{
	--steps_left;
	current[long_axis] += long_sign;
	current[get_other_axis(long_axis)] += slope;
}
