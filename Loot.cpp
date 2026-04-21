#include "Loot.h"

#include "Debug.h"
#include "Item.h"
#include "Random.h"

namespace Loot
{

Ragged<int> s_loot_table; // [ListType][ItemType]

void init()
{
	s_loot_table.resize(ListIndex::Count);

	//									None	Notes	Bean	Potion
	s_loot_table[Empty] = {				1,		0,		0,		0,		};
	s_loot_table[Notes] =			{	0,		1,		0,		0,		};
	s_loot_table[Floor] =			{	0,		0,		13,		1,		};
	s_loot_table[Chest_Main] = {		1,		0,		0,		1,		};
	s_loot_table[Student_Generic] =	{	5,		0,		0,		1,		};

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

Item::Type select(ListIndex list_index)
{
	std::vector<int>& weight_list = s_loot_table.at(list_index);
	int const roll = Random::weighted_index(weight_list);
	
	// Subtract 1 so that None equals -1, not 0.
	return (Item::Type)(roll - 1);
}

} // namespace Loot
