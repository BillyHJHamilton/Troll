#pragma once

#include "Types.h"
#include <string>

namespace Item
{
	enum class UseResult : byte
	{
		Consumed,
		NotConsumed
	};

	enum Type : int
	{
		// This list determines the order items appear in inventory.

		None = c_Invalid,

		// School notes from a character.  Can be used to learn a spell.
		// Subtype: Character Type who wrote it.
		// Flavour: Spell contained in the notes.
		Notes,

		// Bertie Bott's Every Flavour Bean.
		// Flavour: Defined in BertieBotts.cpp
		BBBean,

		// Other miscellaneous sweets.
		// Flavour: Defined in Sweets.h.
		SweetsItem,

		// Flavour: Defined in Potion.h.
		PotionItem,

		Count
	};

	struct Instance
	{
		Type type = None;

		// Item parameters.  Sort of an implicit union.
		// The names are suggestive but the real meaning depends on the item type.
		int subtype = c_Invalid;
		int flavour = c_Invalid;

		// Used only when item is stacked on the ground.
		// Index of the item lying under this one.
		int next = c_Invalid;

		// 1 + number of items below this one.
		int height = 1;

		void serialize(ISerializer& s);
	};

	enum class BagStack : byte
	{
		None = 0,	// Item does not stack in inventory.
		ByType,		// Item will stack with others of same type.
		ByFlavour	// Item will stack with others of same type and flavour.
	};

	// Represents an item instance, as an integer index to the global array.
	// Works the same as Creature::Handle.
	// Note that every Item is potentially a linked list (an "item stack").
	class Handle
	{
		int index;
	public:
		// int interface
		Handle () : index(c_Invalid) { }
		Handle (int const i) : index(i) { }
		explicit operator int () { return index; }
		explicit operator int const () const { return index; }
		bool operator== (Handle rhs) const { return index == rhs.index; }
		bool operator!= (Handle rhs) const { return index != rhs.index; }

		// invalidate handle without destroying item it points to
		void invalidate() { index = c_Invalid; }

		// Simple accessors
		bool valid () const;
		Item::Type type () const;
		bool has_subtype () const { return subtype() != c_Invalid; }
		bool has_flavour () const { return flavour() != c_Invalid; }
		int subtype () const;
		int flavour () const;
		Item::Handle next_in_stack () const;
		int stack_height () const;

		// Complex accessors
		int codepoint () const;
		std::string name () const;
		char const* colour () const;
		std::string description () const;
		std::string interaction_name () const;
		//bool is_plural () const;
		bool can_use () const;
		bool can_discard () const;
		bool can_sell () const;
		BagStack bag_stack_mode () const;
		bool stacks_in_bag () const { return bag_stack_mode() != BagStack::None; }
		bool can_stack_in_bag_with (Item::Handle other) const;
		int buy_price () const;
		int sell_price () const;

		// Mutators
		UseResult use ();
		void stack_onto (Item::Handle other);
		void destroy();
		void destroy_stack();

	protected:
		// Polymorphic stuff
		UseResult use_bbbean();
		UseResult use_sweets();
		UseResult use_notes();
		UseResult use_potion();
	};

	// Global interface
	void init();
	void clear();
	void serialize(ISerializer& s);

	// Create item without adding it to the world.
	Item::Handle make_item (Instance instance);
	Item::Handle make_notes (Creature::Type owner);
	Item::Handle make_bbb ();
	Item::Handle make_sweets ();
	Item::Handle make_potion (Potion::Type potion);
	Item::Handle make_potion_by_level (float difficulty);

	// Make item and parameterize it appropriately.
	Item::Handle make_generic(Item::Type type, Creature::Type creature_type,
		float difficulty);

	// Returns item from top of stack, while redirecting referenced variable to next item down.
	Item::Handle unstack(Item::Handle& item_stack);
};

// Other possible items:
// Camera
// Doxycide
// Flesh-Eating Slug Repellant
// Skele-Gro
