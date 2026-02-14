#include "Inventory.h"

#include "Debug.h"
#include "VectorUtil.h"

//-------------------------------------------------------------------------------------------------
// Data

Inventory s_inventory;

//-------------------------------------------------------------------------------------------------
// Interface functions

Inventory::Inventory()
{
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

int Inventory::num_items () const
{
	return Util::Size(invent);
}

Item::Handle const Inventory::peek_item (int slot) const
{
	if (Util::IsValidIndex(invent, slot))
	{
		return invent.at(slot);
	}
	DebugBreak();
	return c_invalid;
}

void Inventory::add_item (Item::Handle new_item)
{
	if (new_item.stacks_in_bag())
	{
		bool found = false;
		for (int i = 0; i < invent.size(); ++i)
		{
			Item::Handle bag_item = invent[i];
			if (bag_item.type() == new_item.type())
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

