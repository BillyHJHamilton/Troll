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
	bean_bag.reserve(200);
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

void Inventory::add_item (Item::Handle item)
{
	switch(item.type())
	{
		case Item::BBBean:
		{
			bean_bag.push_back(item);
			break;
		}

		default:
		{
			invent.push_back(item);
			invent_sort();
			break;
		}
	}
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
		remove_item(slot);
	}
}

void Inventory::remove_item (int slot)
{
	if (!Util::IsValidIndex(invent, slot))
	{
		DebugBreak();
		return;
	}

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

