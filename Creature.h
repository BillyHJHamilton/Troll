#pragma once

#include "Types.h"
#include "Geometry.h"

#include <iostream>

// Creatures are people and other living beings in the game.
// Most creatures are the "bad guys", but there is also a creature for the player.
// This keeps the code simple as we can use the same functions for player and NPC.
// Concerns that apply *only* to the player, such as XP, will be handled elsewhere.

// Individual creatures are identified by a Creature::Handle throughout the program.
// This can be treated as an int, but also provides an interface for that creature.
// The index of Creature::Player == 0 is reserved for the player.
// Don't store the handle to check a creature's identity, since a removed creature's
// handle may be reused.  If necessary we can add a separate ID for each individual.

// Each creature has a type (Creature::Type) enum.  Don't confuse handle with type.
// There may be multiple creatures of the same type, though hopefully not for named
// characters like Neville or Dumbledore.

struct DrawView;

enum class Gender
{
	Male,
	Female,
	Neuter
};

namespace Creature
{
	enum Type : int
	{
		None = -1,	 // not included in count
		Player = 0,
		Neville_0,
		ColinCreevy_0,
		Count
	};

	struct Stats
	{
		Type type;
		char const * name;
		int codepoint;
		Gender gender;
		int skill_magic;
		int max_hp;

		int hp;
		Vec2 pos;
	};

	struct DerivedStats
	{
		int distractedness;
		int miscastiness;
		int evasion;
		int accuracy;
		int shield_strength;
	};

	// Creature::Handle
	// This is simply a glorified array index that automatically forwards
	// opreations to the correct creature in the creature array.
	// This approach lets us keep const correctness on creature operations.
	class Handle
	{
		int index;
	public:
		// int interface
		Handle () : index(Creature::None) { }
		Handle (int const i) : index(i) { }
		operator int () { return index; }
		operator int const () const { return index; }
		Creature::Handle & operator++ () { ++ index; return *this; }

		// Simple accessors
		bool valid () const;
		Creature::Type type () const;
		std::string name () const;
		Gender gender () const;
		int skill_magic () const;
		int max_hp () const;
		int hp () const;
		Vec2 const & pos () const;
		bool has_status (Status::Index status) const;
		int status_severity (Status::Index status) const;
		int distractedness () const;
		int miscastiness () const;
		int evasion () const;
		int accuracy () const;
		bool knows_spell (Spell::Index spell) const;

		// Complex accessors
		bool is_player () const;
		bool visible () const;
		float miscast_rate_for_spell (Spell::Index spell) const;
		std::string status_string () const;
		std::vector<Spell::Index> spells_known () const;

		// Mutators
		void take_damage (int damage);
		void move (Vec2 const & new_pos);
		void inflict_status (Status::Index status, int severity);
		void reduce_status (Status::Index status, int reduction);
		void cure_status (Status::Index status);
		void cure_all (); // heals status and hp
		
		void update_derived_stats ();
	};

	// Iterator over all valid creatures handles
	class HandleItr
	{
	public:
		HandleItr();
		Creature::Handle get() const;
		void advance();
		bool finished () const;
		int operator++ () { advance(); return current; }
		int operator* () const { return get(); }
		Creature::Handle * operator->() { return &current; }
		explicit operator bool () const { return !finished(); }
	private:
		Creature::Handle current;
	};

	// -----------------------------------------------------------------------------------------------
	// Global interface

	void init ();
	void clear ();

	Creature::Handle creature_at_pos (Vec2 pos);

	Creature::Handle spawn_creature (Creature::Type type, Vec2 const & pos);

	// Visible creature operations
	void update_visible_creatures ();
	void draw_creature (Creature::Handle creature_index, DrawView const & view);
	void draw_visible_creatures (DrawView const & view);
	std::vector<Creature::Handle> const & get_visible_creatures ();
};
