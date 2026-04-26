#pragma once

#include "Types.h"
#include "Inventory.h"

namespace Shop
{
	void clear();
	void serialize(ISerializer& s);

	void restock(float difficulty);
};
