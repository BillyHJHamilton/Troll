#pragma once

#include "Types.h"

// Functions related to selecting types of items to spawn.
// Used for giving items to creatures, placing items in chests, etc.
namespace Loot
{
	enum ListIndex : int
	{
		None = c_Invalid,
		Empty,
		Notes,
		Floor,
		Chest_Main,
		Student_Generic,
		Count
	};

	void init();

	Item::Type select(ListIndex list_index);
};
