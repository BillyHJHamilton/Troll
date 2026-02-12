#include "Item.h"

#include "Debug.h"
#include "VectorUtil.h"
#include "World.h"

namespace Item
{

//-------------------------------------------------------------------------------------------------
// Data

std::vector<Item::Instance> s_items;

//-------------------------------------------------------------------------------------------------
// Helper functions

Item::Instance const& read_inst(int i)
{
	assert(Util::IsValidIndex(s_items, i));
	return s_items[i];
}

Item::Instance& edit_inst(int i)
{
	assert(Util::IsValidIndex(s_items, i));
	return s_items[i];
}

int find_free_index()
{
	for (int i = 0; i < Util::Size(s_items); ++i)
	{
		if (!Item::Handle(i).valid())
		{
			return i;
		}
	}

	s_items.push_back(Item::Instance{});
	return Util::LastIndex(s_items);
}

//-------------------------------------------------------------------------------------------------
// Item Handles

bool Handle::valid() const
{
	return Util::IsValidIndex(s_items, index)
	&& s_items[index].type > Item::None
	&& s_items[index].type < Item::Count;
}

Item::Type Handle::type () const
{
	return read_inst(index).type;
}

Item::Handle Handle::next_in_stack () const
{
	return read_inst(index).next;
}

void Handle::stack_onto (Item::Handle other)
{
	edit_inst(index).next = other;
}

//-------------------------------------------------------------------------------------------------
// Global interface

void init()
{

}

void clear()
{
	s_items.clear();
	s_items.reserve(200); // get us started
}

Item::Handle spawn_item (Instance instance, Vec3 const & pos)
{
	int const new_index = find_free_index();

	Instance& new_inst = s_items[new_index];
	new_inst = instance;

	World::edit().add_item(pos, new_index);

	// Confirm that we added to a valid map.
	assert(World::read().peek_item(pos) == new_index);

	return Item::Handle(new_index);
}


} // namespace Item
