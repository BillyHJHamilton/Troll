#include "Loot.h"

#include "Debug.h"
#include "Item.h"
#include "Random.h"

namespace Loot
{

Ragged<int> s_loot_table; // [ListType][ItemType]

void init()
{
	s_loot_table.resize(Loot::Type::Count);

	//									None	Notes	Bean	Sweets	Potion
	s_loot_table[Empty] =			{	1,		0,		0,		0,		0,		};
	s_loot_table[Notes] =			{	0,		1,		0,		0,		0,		};
	s_loot_table[Sweets] =			{	0,		0,		0,		1,		0,		};
	s_loot_table[Potion] =			{	0,		0,		0,		0,		1,		};
	s_loot_table[Floor] =			{	0,		0,		14,		3,		1,		};
	s_loot_table[Chest_Main] =		{	0,		0,		0,		1,		2,		};
	s_loot_table[Student_Generic] =	{	8,		0,		0,		3,		1,		};

	// Validation
	int constexpr c_ExpectedSize = Item::Count + 1;
	for (std::vector<int>& list : s_loot_table)
	{
		if (Util::Size(list) != c_ExpectedSize)
		{
			DebugBreak("Loot list is missing or wrong size.");
		}
	}
}

Item::Type select(Loot::Type loot_type)
{
	std::vector<int>& weight_list = s_loot_table.at(loot_type);
	int const roll = Random::weighted_index(weight_list);
	
	// Subtract 1 so that None equals -1, not 0.
	return (Item::Type)(roll - 1);
}

Item::Type select(Loot::TypePercent type_percent)
{
	if (Random::in_range(0,99) >= type_percent.percent)
	{
		return Item::None;
	}
	else
	{
		return select(type_percent.type);
	}
}


} // namespace Loot
