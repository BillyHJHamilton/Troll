#pragma once

#include "Types.h"

// Functions related to selecting types of items to spawn.
// Used for giving items to creatures, placing items in chests, etc.
namespace Loot
{
	enum Type : int
	{
		None = c_Invalid,
		Empty,
		Notes,
		Sweets,
		Potion,
		Floor,
		Chest_Main,
		Student_Generic,
		Count
	};

	struct TypePercent
	{
		Type type;
		int percent;
	};

	void init();

	Item::Type select(Loot::Type type);
	Item::Type select(Loot::TypePercent type_percent);
};
