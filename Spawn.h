#pragma once

#include "Types.h"

// Deals with placing characters and items in the world.
namespace Spawn
{
	void clear();
	void serialize(ISerializer& s);

	void post_world_setup();
	void check_spawning();
}
