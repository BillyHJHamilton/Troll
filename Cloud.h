#pragma once

#include "Types.h"

namespace Cloud
{
	enum Type : int
	{
		None = -1,
		Smoke = 0,
		Slime,
		Puddle,
		Count,
	};

	// TODO Could create "Puddles" (~) separate from clouds.
	// The main benefit would be that smoke could conceal a slime puddle.
	// In future, clouds/puddles also might behave different in water.

	inline bool is_cloud (Cloud::Type c) { return c >= Cloud::Type(0) && c < Count; }
	int get_codepoint (Cloud::Type c);
	char const * get_colour (Cloud::Type c);
	char const * look_describe(Cloud::Type c);
	int accuracy_loss (Cloud::Type c);
	int vision_loss (Cloud::Type c);

	//bool affects_creatures (Cloud::Type cloud);
	bool hazardous_for (Cloud::Type cloud, Creature::Handle const& creature);
	void affect_creature (Cloud::Type cloud, Creature::Handle& creature);
}
