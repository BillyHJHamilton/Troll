#pragma once

#include "Item.h"
#include "Types.h"

class Inventory
{
public:
	Inventory();
	static void clear();
	static Inventory& edit();
	static Inventory const& read();

	int num_items () const;
	Item::Handle const peek_item (int slot) const;

	void add_item (Item::Handle item);
	void use_item (int slot);
	void remove_item (int slot);

protected:
	void invent_sort ();

	std::vector<Item::Handle> invent;
};
