#pragma once

#include "Types.h"

namespace Cloud
{
	enum Type : int
	{
		None = -1,
		Smoke = 0,
		Count,
	};

	bool is_cloud (Cloud::Type c) { return c >= Cloud::Type(0) && c < Count; }
	int get_character (Cloud::Type c);
	char const * get_colour (Cloud::Type c);
	int accuracy_loss (Cloud::Type c);
	int vision_loss (Cloud::Type c);
}
