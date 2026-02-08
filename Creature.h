#pragma once

#include "Types.h"
#include "Geometry.h"
#include "NameHash.h"

#include <iostream>

// Creatures are people and other living beings in the game.
// Most creatures are the "bad guys", but there is also a creature for the player.
// This keeps the code simple as we can use the same functions for player and NPC.
// Concerns that only apply to the player, like XP, will be in the Player module.
// Concerns for AI that only apply to NPCs are in the Bot module.

// Individual creatures are identified by a Creature::Handle throughout the program.
// This can be treated as an int, but also provides an interface for that creature.
// The index of Creature::Player == 0 is reserved for the player.
// Don't store the handle to check a creature's identity, since a removed creature's
// handle may be reused.  If necessary we can add a separate ID for each individual.

// Each creature has a type (Creature::Type) enum.  Don't confuse handle with type.
// There may be multiple creatures of the same type, though hopefully not for named
// characters like Neville or Dumbledore.

static bool constexpr SHOW_CREATURE_DEBUG = true;

enum class Gender : byte
{
	Male,
	Female,
	Neuter
};

// Walk movement can be disrupted by the walk_failure stat.
// Forced movement is not disrupted in this way.
enum class MoveMode : byte
{
	Walk,
	Forced
};

namespace Creature
{
	enum Type : int
	{
		None = -1,	 // not included in count
		Player = 0,

		Neville_0,
		ColinCreevy_0,
		SallyAnne_0,

		Harry_1,
		Malfoy_1,

		Ron_2,
		Hermione_2,

		Crabbe_3,
		Goyle_3,

		Hufflepuff_1,

		Count
	};

	enum class Identity : int
	{
		Generic = -1,
		Player = 0,

		// Alphabetic by first name below this point
		ColinCreevy,
		DracoMalfoy,
		GregoryGoyle,
		HarryPotter,
		HermioneGranger,
		NevilleLongbottom,
		RonWeasley,
		SallyAnnePerks,
		VincentCrabbe,

		Count
	};

	struct Stats
	{
		Identity identity = Identity::Generic;
		float difficulty = 0.0f;
		float probability = 1.0f;
		char const * short_name = nullptr;
		char const * long_name = nullptr;
		char const * colour = nullptr;
		int codepoint = 0; // letter to display
		int skill_magic = 0;
		int max_hp = 0;
		Gender gender = Gender::Male;
	};

	struct Instance
	{
		Type type = Creature::None;
		int hp = 0;
		Vec3 pos = {0,0,0};
		int rest_turns = 0; // counter for healing by resting
	};

	struct DerivedStats
	{
		int distractedness = 0;
		int miscastiness = 0;
		int evasion = 0;
		int accuracy = 0;
		int shield_strength = 0;
		int walk_failure = 0;
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
		Creature::Identity identity () const;
		bool is_generic () const;
		std::string short_name () const;
		std::string long_name () const;
		Gender gender () const;
		int skill_magic () const;
		int max_hp () const;
		int hp () const;
		float hp_percent() const;
		bool is_hurt() const;
		Vec3 pos () const;
		bool has_status (Status::Index status) const;
		int status_severity (Status::Index status) const;
		int distractedness () const;
		int miscastiness () const;
		int evasion () const;
		int accuracy () const;
		int walk_failure () const;
		bool knows_spell (Spell::Index spell) const;
		bool has_tag (NameHash tag) const;

		// Complex accessors
		bool is_player () const;
		bool visible () const;
		float miscast_rate_for_spell (Spell::Index spell) const;
		std::string status_string () const;
		std::vector<Spell::Index> spells_known () const;

		// Mutators
		void take_damage (int damage, Creature::Handle instigator);
		void move (Vec3 const & new_pos);
		void inflict_status (Status::Index status, int severity);
		void reduce_status (Status::Index status, int reduction);
		void cure_status (Status::Index status);
		void cure_all (); // heals status and hp
		void rest_step ();
		void clear_rest_steps ();
		void invalidate ();
		
		void update_derived_stats ();
	};

	// Iterator over all valid creatures handles
	class HandleItr
	{
	public:
		HandleItr(int start_at); // start at 0 to include player, or 1 to skip player
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

	void mix_gingerbread (
		Creature::Type type, Creature::Identity identity, float difficulty, float probability,
		char const * short_name, char const * long_name,
		int codepoint, char const * colour, Gender gender,
		int magic_skill, int max_hp, std::string spell_string,
		char const * tag_string = "");
	void init_gingerbread();

	Stats& edit_player_stats();

	const char* short_name_from_type(Creature::Type type);
	const char* long_name_from_type(Creature::Type type);
	Creature::Handle creature_at_pos (Vec3 pos);
	bool is_anyone_at (Vec3 pos);

	Creature::Handle spawn_creature (Creature::Type type, Vec3 const & pos);

	Creature::Type find_type_to_spawn (float target_difficulty);

	// Visible creature operations
	void update_visible_creatures ();
	void draw_creature (Creature::Handle creature_index, Draw::View const & view);
	void draw_visible_creatures (Draw::View const & view);
	void remove_defeated_creatures ();
	std::vector<Creature::Handle> const & get_visible_creatures ();
};
