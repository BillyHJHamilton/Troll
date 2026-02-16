#include "Item.h"

#include "BertieBotts.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Gingerbread.h"
#include "Player.h"
#include "Potion.h"
#include "Random.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

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
// Item Handle Interface

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

int Handle::subtype () const
{
	return read_inst(index).subtype;
}

int Handle::flavour () const
{
	return read_inst(index).flavour;
}

Item::Handle Handle::next_in_stack () const
{
	return read_inst(index).next;
}

int Handle::stack_height () const
{
	return read_inst(index).height;
}

int Handle::codepoint () const
{
	switch (type())
	{
		case BBBean:		return ',';
		case PotionItem:	return '!';
		default:			return '?';
	}
}

std::string Handle::name () const
{
	switch (type())
	{
		case Notes:
		{
			Creature::Type const owner = (Creature::Type)(read_inst(index).subtype);
			return Gingerbread::short_name(owner) + "'s notes";
		}

		case BBBean:
			return "Bertie Bott's Every Flavour Bean";

		case PotionItem:
			return Potion::get_name(flavour());

		default:
			DebugBreak();
			return "";
	}
}

std::string Handle::colour () const
{
	switch (type())
	{
		case BBBean:
			return BertieBotts::get_colour(flavour());

		case PotionItem:
			return Potion::get_colour(flavour());

		default:
			return "white";
	}
}

std::string Handle::description () const
{
	switch (type())
	{
		case Notes:
		{
			Creature::Type const owner = (Creature::Type)(read_inst(index).subtype);
			return std::format("Some parchment covered in {}'s handwriting.",
				Gingerbread::short_name(owner));
		}

		case BBBean:
		{
			return "They mean every flavour.";
		}

		case PotionItem:
		{
			return Potion::get_description(flavour());
		}

		default:
		{
			return "";
		}
	}
}

std::string Handle::interaction_name () const
{
	switch (type())
	{
		case Notes:
			return "Read";

		case BBBean:
			return "Eat";

		case PotionItem:
			return "Imbibe";

		default:
			return "Use";
	}
}

bool Handle::can_use () const
{
	// For now
	return true;
}

bool Handle::can_discard () const
{
	// For now
	return true;
}

Item::BagStack Handle::bag_stack_mode () const
{
	switch (type())
	{
		case BBBean:
			return BagStack::ByType;

		case PotionItem:
			return BagStack::ByFlavour;

		default:
			return BagStack::None;
	}
}

bool Handle::can_stack_in_bag_with (Item::Handle other) const
{
	if (other.type() == type())
	{
		if (bag_stack_mode() == BagStack::ByType)
		{
			return true;
		}

		if (other.flavour() == flavour()
			&& bag_stack_mode() == BagStack::ByFlavour)
		{
			return true;
		}
	}

	return false;
}

UseResult Handle::use ()
{
	switch (type())
	{
		case Notes:			return use_notes();
		case BBBean:		return use_bbbean();
		case PotionItem:	return use_potion();

		default:
			DebugBreak();
			return UseResult::NotConsumed;
	}
}

void Handle::stack_onto (Item::Handle other)
{
	edit_inst(index).next = other;

	edit_inst(index).height = other.valid() ?
		1 + other.stack_height() :
		1;
}

void Handle::invalidate()
{
	edit_inst(index) = {};
}

void Handle::invalidate_stack()
{
	Item::Handle next = *this;
	while (next.valid())
	{
		Item::Handle target = next;
		next = next.next_in_stack();
		target.invalidate();
	}
}

//-------------------------------------------------------------------------------------------------
// Item Handle Helpers

UseResult Handle::use_notes()
{
	Draw::add_message("You peruse " + name() + ".");
	if (has_flavour())
	{
		Spell::Index spell = (Spell::Index)flavour();
		if (!Spell::is_valid_index(spell))
		{
			DebugBreak();
			return UseResult::Consumed;
		}

		Draw::add_message(" It's all about the " + Spell::get_name(spell) + " spell.");

		if (Player::handle().knows_spell(spell))
		{
			Draw::add_message(" But you already know that one.");
		}
		else
		{
			// TODO: This should really be a longer action, not safe to use in combat.

			Draw::add_message(" Learned to cast " + Spell::get_name(spell) + "!");
			Player::handle().learn_spell(spell);
		}
		return UseResult::Consumed;
	}

	return UseResult::Consumed;
}

UseResult Handle::use_bbbean()
{
	Draw::add_message("You eat a bean.");
	Draw::add_message(BertieBotts::eat_message(flavour()));
	return UseResult::Consumed;
}

UseResult Handle::use_potion()
{
	Draw::add_message(std::format("You drink the {}.", name()));
	Potion::drink(Player::handle(), flavour());
	return UseResult::Consumed;
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

Item::Handle spawn_bbb (Vec3 pos)
{
	Item::Instance inst;
	inst.type = Item::BBBean;
	inst.flavour = BertieBotts::random_flavour();
	return Item::spawn_item(inst, pos);
}

Item::Handle spawn_notes (Vec3 pos, Creature::Type owner_type)
{
	Item::Instance inst;
	inst.type = Item::Notes;

	inst.subtype = (int)owner_type;

	std::vector<Spell::Index> spells;
	Spell::bitset_to_list(Gingerbread::read_spells(owner_type), spells);
	inst.flavour = (int)Random::from_vector(spells);

	return Item::spawn_item(inst, pos);
}

Item::Handle spawn_potion (Vec3 pos, Potion::Type potion)
{
	Item::Instance inst;
	inst.type = Item::PotionItem;
	inst.flavour = potion;
	return Item::spawn_item(inst, pos);
}

} // namespace Item
