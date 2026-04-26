#include "Shop.h"

#include "Loot.h"
#include "Random.h"

namespace Shop
{

//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ShopItems = 7;

Inventory s_shop_inventory;

//-------------------------------------------------------------------------------------------------
// Interface

void clear()
{
	s_shop_inventory = Inventory{};
}

void serialize(ISerializer& s)
{
	s_shop_inventory.serialize_instance(s);
}

void restock(float difficulty)
{
	// First clean up some old inventory.
	for (int i = 0; i < s_shop_inventory.num_slots(); ++i)
	{
		int const n = s_shop_inventory.peek_item(i).stack_height();
		if (n == 1)
		{
			if (Random::coinflip())
			{
				s_shop_inventory.remove_item(i);
				--i;
				continue;
			}
		}
		else
		{
			int const num_to_remove = Random::in_range(1, n);
			s_shop_inventory.remove_items(i, num_to_remove);
		}
	}

	// Now add new items up to the total.
	int const total_left = s_shop_inventory.total_items();
	int const num_to_add = c_ShopItems - total_left;
	for (int i = 0; i < num_to_add; ++i)
	{
		Item::Handle new_item = Loot::make(Loot::Shop, (Creature::Type)c_Invalid, difficulty);
		s_shop_inventory.add_item(new_item);
	}
}

}
