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
	s_inventory.serialize_instance(s);
}

void Inventory::serialize_instance(ISerializer& s)
{
	s.srz_vector(invent, "s_inventory");
}

bool Inventory::has_item () const
{
	return !invent.empty();
}

int Inventory::num_slots () const
{
	return Util::Size(invent);
}

int Inventory::total_items () const
{
	int n = 0;
	for (int i = 0; i < num_slots(); ++i)
	{
		n += invent[i].stack_height();
	}
	return n;
}

int Inventory::num_beans () const
{
	int const bean_slot = find_first_item(Item::BBBean);
	return (bean_slot == c_Invalid) ?
		0 :
		peek_item(bean_slot).stack_height();
}

int Inventory::random_slot () const
{
	if (num_slots() == 0)
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

int Inventory::find_first_item(Item::Type type) const
{
	if (type == c_Invalid || type >= Item::Count)
	{
		return c_Invalid;
	}

	for (int i = 0; i < Util::Size(invent); ++i)
	{
		Item::Handle const& item = invent[i];
		if (item.type() == type)
		{
			return i;
		}
	}

	return c_Invalid;
}

int Inventory::find_most_recently_used() const
{
	if (!has_recent_type())
	{
		return c_Invalid;
	}

	for (int i = 0; i < Util::Size(invent); ++i)
	{
		Item::Handle const& item = invent[i];
		if (item.type() == recent_type)
		{
			if (item.bag_stack_mode() == Item::BagStack::ByType)
			{
				return i;
			}

			if (item.bag_stack_mode() == Item::BagStack::ByFlavour &&
				item.flavour() == recent_flavour)
			{
				return i;
			}
		}
	}

	return c_Invalid;
}

bool Inventory::has_recent_type() const
{
	return recent_type != Item::None;
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

	recent_type = invent[slot].type();
	recent_flavour = invent[slot].flavour();

	Item::UseResult result = invent[slot].use();
	if (result == Item::UseResult::Consumed)
	{
		if (invent[slot].stack_height() > 1)
		{
			Item::Handle used = invent[slot];
			invent[slot] = used.next_in_stack();
			used.destroy();
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

	invent[slot].destroy_stack();
	Util::RemoveAt(invent, slot);
}

void Inventory::remove_items (int slot, int num_to_remove)
{
	if (!Util::IsValidIndex(invent, slot))
	{
		DebugBreak();
		return;
	}

	if (num_to_remove == invent[slot].stack_height())
	{
		invent[slot].destroy_stack();
		Util::RemoveAt(invent, slot);
	}
	else
	{
		for (int i = 0; i < num_to_remove; ++i)
		{
			Item::Handle item = Item::unstack(invent[slot]);
			item.destroy();
		}
	}
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

