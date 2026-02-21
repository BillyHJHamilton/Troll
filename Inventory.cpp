#include "Inventory.h"

#include "Debug.h"
#include "Random.h"
#include "Serialize.h"
#include "VectorUtil.h"

//-------------------------------------------------------------------------------------------------
// Data

Inventory s_inventory;

//-------------------------------------------------------------------------------------------------
// Interface functions

Inventory::Inventory()
{
	// Enough for now, given the way they stack!
	invent.reserve(200);
}

void Inventory::clear()
{
	s_inventory = Inventory{};
}

Inventory& Inventory::edit()
{
	return s_inventory;
}

Inventory const& Inventory::read()
{
	return s_inventory;
}

void Inventory::serialize(ISerializer& s)
{
	srz_vector(s, s_inventory.invent, "s_inventory");
}

bool Inventory::has_item () const
{
	return !invent.empty();
}

int Inventory::num_items () const
{
	return Util::Size(invent);
}

int Inventory::random_slot () const
{
	if (num_items() == 0)
	{
		return c_Invalid;
	}
	else
	{
		return Random::index(invent);
	}
}

Item::Handle const Inventory::peek_item (int slot) const
{
	if (Util::IsValidIndex(invent, slot))
	{
		return invent.at(slot);
	}
	DebugBreak();
	return c_Invalid;
}

void Inventory::add_item (Item::Handle new_item)
{
	// Item must be removed from world stack before being added to bag.
	// Otherwise the world stack and bag stack will get joined!
	assert(new_item.next_in_stack() == c_Invalid);

	if (new_item.stacks_in_bag())
	{
		bool found = false;
		for (int i = 0; i < invent.size(); ++i)
		{
			Item::Handle bag_item = invent[i];
			if (new_item.can_stack_in_bag_with(bag_item))
			{
				new_item.stack_onto(bag_item);
				invent[i] = new_item;
				found = true;
				break;
			}
		}

		if (!found)
		{
			invent.push_back(new_item);
		}
	}
	else
	{
		invent.push_back(new_item);
	}

	invent_sort();
}

Item::Handle Inventory::pop_item (int slot)
{
	if (!Util::IsValidIndex(invent, slot))
	{
		DebugBreak();
		return Item::Handle(c_Invalid);
	}

	Item::Handle item = Item::unstack(invent[slot]);

	if (!invent[slot].valid())
	{
		Util::RemoveAt(invent, slot);
	}

	return item;
}

void Inventory::use_item (int slot)
{
	if (!Util::IsValidIndex(invent, slot))
	{
		DebugBreak();
		return;
	}

	Item::UseResult result = invent[slot].use();
	if (result == Item::UseResult::Consumed)
	{
		if (invent[slot].stack_height() > 1)
		{
			Item::Handle used = invent[slot];
			invent[slot] = used.next_in_stack();
			used.invalidate();
		}
		else
		{
			remove_item(slot);
		}
	}
}

void Inventory::remove_item (int slot)
{
	if (!Util::IsValidIndex(invent, slot))
	{
		DebugBreak();
		return;
	}

	// TODO: Better deallocate it!!
	// TODO: Better confirm it's not a lot of items deep in a stack!

	invent[slot].invalidate_stack();
	Util::RemoveAt(invent, slot);
}

//-------------------------------------------------------------------------------------------------
// Helper functions

void Inventory::invent_sort ()
{
	auto func = [](Item::Handle a, Item::Handle b)
	{
		if (a.type() != b.type())
		{
			return a.type() < b.type();
		}
		else if (a.has_subtype() && b.has_subtype() && a.subtype() != b.subtype())
		{
			return a.subtype() < b.subtype();
		}
		else if (a.has_flavour() && b.has_flavour() && a.flavour() != b.flavour())
		{
			return a.flavour() < b.flavour();
		}
		else
		{
			// Shrug!
			return a < b;
		}
	};

	std::sort(invent.begin(), invent.end(), func);
}

