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

		None = c_invalid,

		// School notes from a character.  Can be used to learn a spell.
		// subtype: Character Type
		// flavour: Spell contained in the notes
		Notes,

		// Bertie Bott's Every Flavour Bean.
		// flavour: As defined in BertieBotts.cpp
		BBBean,

		Count
	};

	struct Instance
	{
		Type type = None;

		// Item parameters.  Sort of an implicit union.
		// The names are suggestive but the real meaning depends on the item type.
		int subtype = c_invalid;
		int flavour = c_invalid;

		// Used only when item is stacked on the ground.
		// Index of the item lying under this one.
		int next = c_invalid;

		// 1 + number of items below this one.
		int height = 1;
	};

	// Represents an item instance, as an integer index to the global array.
	// Works the same as Creature::Handle.
	class Handle
	{
		int index;
	public:
		// int interface
		Handle () : index(c_invalid) { }
		Handle (int const i) : index(i) { }
		operator int () { return index; }
		operator int const () const { return index; }

		// Simple accessors
		bool valid () const;
		Item::Type type () const;
		bool has_subtype () const { return subtype() != c_invalid; }
		bool has_flavour () const { return flavour() != c_invalid; }
		int subtype () const;
		int flavour () const;
		Item::Handle next_in_stack () const;
		int stack_height () const;

		// Complex accessors
		int codepoint () const;
		std::string name () const;
		std::string colour () const;
		std::string description () const;
		std::string interaction_name () const;
		bool can_use () const;
		bool can_discard () const;
		bool stacks_in_bag () const;

		// Mutators
		UseResult use ();
		void stack_onto (Item::Handle other);
		void invalidate();
		void invalidate_stack();

	protected:
		// Polymorphic stuff
		UseResult use_bbbean();
		UseResult use_notes();
	};

	// Global interface
	void init();
	void clear();

	Item::Handle spawn_item (Vec3 pos, Instance instance);

	Item::Handle spawn_bbb (Vec3 pos);
	Item::Handle spawn_notes (Vec3 pos, Creature::Type owner);
};
