#pragma once

#include "Types.h"
#include "Inventory.h"

namespace Shop
{
	void clear();
	void serialize(ISerializer& s);

	Inventory const& read_inventory();
	Inventory& edit_inventory();

	void restock(float difficulty);

	bool is_active();
	bool try_spawn(Vec3 at);
	void update();
	void interact(Creature::Type shop_creature);
};
