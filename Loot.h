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
		Bean,
		Sweets,
		Potion,
		Floor,
		Treasure,
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

	Item::Handle make(Loot::Type type, Creature::Type creature_type, float difficulty);
	Item::Handle make(Loot::TypePercent type_percent, Creature::Type creature_type,
		float difficulty);

	void spawn(Loot::Type type, Vec3 pos, Creature::Type creature_type, float difficulty);

	// Stacks the new item onto stack_top, and redirects stack_top to point to the new top.
	void stack(Loot::Type type, Item::Handle& stack_top, Creature::Type creature_type,
		float difficulty); 
};
