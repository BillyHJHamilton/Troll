#pragma once

#include "Types.h"

namespace Item
{
	enum Type : int
	{
		None = c_invalid,
		Notes,
		Sweets,
		Count
	};

	struct Instance
	{
		Type type = None;

		// Item parameters.  Sort of an implicit union.
		// The names are suggestive but the real meaning depends on the item type.
		int subtype = c_invalid;
		int flavour = c_invalid;
		int ammo = c_invalid;

		// Used only when item is stacked on the ground.
		// Index of the item lying under this one.
		int next = c_invalid;
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
		Item::Handle next_in_stack () const;

		int get_codepoint () const;
		char const * get_name () const;
		char const * get_colour () const;

		// Mutators

		// Should only be called by Map::add_item
		void stack_onto (Item::Handle other);
	};

	// Global interface
	void init();
	void clear();

	Item::Handle spawn_item (Instance instance, Vec3 const & pos);
};
