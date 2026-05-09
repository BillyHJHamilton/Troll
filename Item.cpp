#include "Item.h"

#include "BertieBotts.h"
#include "Colour.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Gingerbread.h"
#include "Player.h"
#include "Potion.h"
#include "Random.h"
#include "Serialize.h"
#include "Sweets.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

namespace Item
{

//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ItemReserveSize = 1'000;
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
		case SweetsItem:	return '=';
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
			if (owner == Creature::None)
			{
				return "Notes on " + Spell::get_name((Spell::Index)flavour());
			}
			else
			{
				return Gingerbread::short_name(owner) + "'s notes";
			}
		}

		case BBBean:
			return "Bertie Bott's Every Flavour Bean";

		case SweetsItem:
			return Sweets::get_name(flavour());

		case PotionItem:
			return Potion::get_name(flavour());

		default:
			DebugBreak();
			return "";
	}
}

char const* Handle::colour () const
{
	switch (type())
	{
		case BBBean:
			return BertieBotts::get_colour(flavour());

		case SweetsItem:
			return Sweets::get_colour(flavour());

		case PotionItem:
			return Potion::get_colour(flavour());

		default:
			return cstr_White;
	}
}

std::string Handle::description () const
{
	switch (type())
	{
		case Notes:
		{
			Creature::Type const owner = (Creature::Type)(read_inst(index).subtype);
			if (owner == Creature::None)
			{
				return std::format("Some notes describing the {} spell.",
					Spell::get_name((Spell::Index)flavour()));
			}
			else
			{
				return std::format("Some parchment covered in {}'s handwriting.",
					Gingerbread::short_name(owner));
			}
		}

		case BBBean:
		{
			return "They mean every flavour!  "
				"Eating beans may raise your sugar level, depending on the flavour.  "
				"Hogwarts students sometimes use these beans as currency.";
		}

		case SweetsItem:
		{
			return Sweets::get_description(flavour());
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
		case SweetsItem:
			return "Eat";

		case PotionItem:
			return "Imbibe";

		default:
			return "Use";
	}
}

/*bool Handle::is_plural () const
{
	switch (type())
	{
		case Notes:
			return true;

		default:
			return false;
	}
}*/

bool Handle::can_use () const
{
	// For now
	return true;
}

bool Handle::can_discard () const
{
	// For now
	return false;
}

bool Handle::can_sell () const
{
	switch (type())
	{
		case BBBean:
		case Notes:
			return false;

		case SweetsItem:
		case PotionItem:
			return true;

		default:
			DebugBreak("Unhandled case.");
			return false;
	}
}

Item::BagStack Handle::bag_stack_mode () const
{
	switch (type())
	{
		case BBBean:
			return BagStack::ByType;

		case SweetsItem:
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

int Handle::buy_price () const
{
	switch (type())
	{
		case Notes:			return Spell::get_difficulty((Spell::Index)flavour());
		case BBBean:		return 1;
		case SweetsItem:	return Sweets::buy_price(flavour());
		case PotionItem:	return Potion::buy_price(flavour());

		default:
			return c_Invalid;
	}
}

int Handle::sell_price () const
{
	return std::max(1, (2*buy_price())/3);
}

UseResult Handle::use ()
{
	switch (type())
	{
		case Notes:			return use_notes();
		case BBBean:		return use_bbbean();
		case SweetsItem:	return use_sweets();
		case PotionItem:	return use_potion();

		default:
			DebugBreak();
			return UseResult::NotConsumed;
	}
}

void Handle::stack_onto (Item::Handle other)
{
	edit_inst(index).next = (int)other;

	edit_inst(index).height = other.valid() ?
		1 + other.stack_height() :
		1;
}

void Handle::destroy()
{
	edit_inst(index) = {};
}

void Handle::destroy_stack()
{
	Item::Handle next = *this;
	while (next.valid())
	{
		Item::Handle target = next;
		next = next.next_in_stack();
		target.destroy();
	}
}

//-------------------------------------------------------------------------------------------------
// Item Handle Helpers

UseResult Handle::use_notes()
{
	Draw::add_message("You peruse " + name() + ".");
	Draw::IndentScope indent;

	if (has_flavour())
	{
		Spell::Index spell = (Spell::Index)flavour();
		if (!Spell::is_valid_index(spell))
		{
			DebugBreak();
			return UseResult::Consumed;
		}

		Draw::add_message("It's all about the " + Spell::get_name(spell) + " spell.");

		if (Player::handle().knows_spell(spell))
		{
			Draw::add_message("But you already know that one.");
		}
		else
		{
			// TODO: This should really be a longer action, not safe to use in combat.

			Draw::add_message("Learned to cast " + Spell::get_name(spell) + "!");
			Player::handle().learn_spell(spell);
		}
		return UseResult::Consumed;
	}

	return UseResult::Consumed;
}

UseResult Handle::use_bbbean()
{
	Draw::add_message("You eat a bean.  " + BertieBotts::eat_message(flavour()));

	int const amount = BertieBotts::random_sugar_amount(flavour());
	Player::gain_sugar(amount);

	return UseResult::Consumed;
}

UseResult Handle::use_sweets()
{
	Draw::add_message(std::format("You eat the {}.", name()));
	Draw::IndentScope indent;
	Sweets::eat(Player::handle(), flavour());
	return UseResult::Consumed;
}

UseResult Handle::use_potion()
{
	Draw::add_message(std::format("You drink the {}.", name()));
	Draw::IndentScope indent;
	Potion::drink(Player::handle(), flavour());
	return UseResult::Consumed;
}

//-------------------------------------------------------------------------------------------------
// Global interface

void init()
{
	s_items.reserve(c_ItemReserveSize);
}

void clear()
{
	s_items.clear();
}

void Instance::serialize(ISerializer& s)
{
	s.srz_value(type);
	s.srz_int(subtype);
	s.srz_int(flavour);
	s.srz_int(next);
	s.srz_int(height);
}

void serialize(ISerializer& s)
{
	s.srz_vector_advanced(s_items, "s_items");
}

Item::Handle make_item (Instance instance)
{
	int const new_index = find_free_index();

	Instance& new_inst = s_items[new_index];
	new_inst = instance;
	return Item::Handle(new_index);
}

Item::Handle make_notes (Creature::Type owner_type)
{
	Item::Instance inst;
	inst.type = Item::Notes;

	inst.subtype = (int)owner_type;

	const Spell::Bitset& bitset = Gingerbread::read_spells(owner_type);
	Spell::TempList spells = Spell::bitset_to_temp_list(bitset);
	Spell::Index spell = Random::from_vector(spells);

	if (Debug::enabled(Debug::Item))
	{
		std::cout << std::format("{}'s Notes: Selected {}.\n",
			Gingerbread::read(owner_type).short_name, Spell::get_name(spell));
	}

	// Try not to choose Vermillious and Flipendo since they are so ubiquitous
	// and the player probably knows them already;
	for (int i = 0;
		i < 2 && (spell == Spell::Flipendo || spell == Spell::Vermillious);
		++i)
	{
		spell = Random::from_vector(spells);

		if (Debug::enabled(Debug::Item))
		{
			std::cout << std::format(" - Rerolling, selected {}.\n",
				Spell::get_name(spell));
		}
	}

	inst.flavour = (int)spell;

	return make_item(inst);
}

Item::Handle make_bbb ()
{
	Item::Instance inst;
	inst.type = Item::BBBean;
	inst.flavour = BertieBotts::random_flavour();
	return make_item(inst);
}

Item::Handle make_sweets ()
{
	Item::Instance inst;
	inst.type = Item::SweetsItem;
	inst.flavour = Sweets::random_flavour();
	return make_item(inst);
}

Item::Handle make_potion (Potion::Type potion)
{
	Item::Instance inst;
	inst.type = Item::PotionItem;
	inst.flavour = potion;
	return make_item(inst);
}

Item::Handle make_potion_by_level (float difficulty)
{
	return make_potion(Potion::random_by_level(difficulty));
}

Item::Handle make_generic(Item::Type type, Creature::Type creature_type,
	float difficulty)
{
	switch(type)
	{
		case Item::None: return c_Invalid;
		case Item::BBBean: return make_bbb();
		case Item::SweetsItem: return make_sweets();
		case Item::PotionItem: return make_potion_by_level(difficulty);

		case Item::Notes:
			assert(Creature::is_valid_type(creature_type));
			return make_notes(creature_type);

		default:
			DebugBreak("Unhandled case");
			return c_Invalid;
	}
}

Item::Handle unstack(Item::Handle& item_stack)
{
	if (item_stack.valid())
	{
		Item::Handle top = item_stack;
		item_stack = top.next_in_stack();
		top.stack_onto(c_Invalid);
		return top;
	}
	return Item::Handle(c_Invalid);
}

} // namespace Item
