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
	static void serialize(ISerializer& s);

	bool has_item () const;
	int num_items () const;
	int random_slot () const;
	Item::Handle const peek_item (int slot) const;

	// Find first slot with item of the given type.
	int find_first_item(Item::Type type) const;

	// Finds the current slot of the most recently used item type.
	// Returns c_Invalid if we have no more of that item.
	int find_most_recently_used() const;

	void add_item (Item::Handle item);
	void use_item (int slot);
	void remove_item (int slot); // removes entire slot
	Item::Handle pop_item (int slot); // for stealing from player

protected:
	void invent_sort ();

	std::vector<Item::Handle> invent;

	// Tracking most recently used item
	Item::Type recent_type = Item::None;
	int recent_flavour = c_Invalid;
};
