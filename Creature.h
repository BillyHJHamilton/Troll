#pragma once

#include "Types.h"

#include "Geometry.h"
#include "Item.h"
#include "NameHash.h"
#include "Scratch.h"
#include "Spell.h"

#include <iostream>

// Creatures are people and other living beings in the game.
// Most creatures are the "bad guys", but there is also a creature for the player.
// This keeps the code simple as we can use the same functions for player and NPC.

// Individual creatures are identified by a Creature::Handle throughout the program.
// This can be treated as an int, but also provides an interface for that creature.
// The index of Creature::Player == 0 is reserved for the player.
// Don't store the handle to check a creature's identity, since a removed creature's
// handle may be reused.  If necessary we can add a separate ID for each individual.

// Each creature has a type (Creature::Type) enum.  Don't confuse handle with type.
// There may be multiple creatures of the same type, though normally not for named
// characters like Neville or Dumbledore.

// Related modules:
//  - Player - Values that apply only to the player, like XP.
//  - Bot - Creature behaviour and AI related data
//  - Gingerbread - Invariant data about a creature type
//  - Squad - Groups of creatures spawned together

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
	int constexpr c_MaxCreatures = 200;
	int constexpr c_RestTurnsPerHp = 5;

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

		Harry_4,
		Cedric_4,
		Fleur_4,

		Krum_5,
		Neville_5,
		Ginny_5,
		Luna_5,

		MarySue,

		// Generic Students
		Hufflepuff_1,
		Ravenclaw_1,
		Gryffindor_1,
		Slytherin_1,

		Hufflepuff_2,
		Ravenclaw_2,
		Gryffindor_2,
		Slytherin_2,

		Hufflepuff_3,
		Ravenclaw_3,
		Gryffindor_3,
		Slytherin_3,

		// Fantastic Beasts and Where to Find Them
		Gnome,
		Streeler,
		FireCrab,
		BigFireCrab,
		Doxy,
		Imp,

		// AU's!
		HarryTheHufflepuff_1,

		Count
	};

	using TypeList = std::vector<Creature::Type>;
	using TypeTempList = std::vector<Creature::Type, Scratch<Creature::Type>>;

	// Creature Habitats represent where a creature can spawn.
	// They include other categories used to choose which creature can spawn.
	// Eventually there should be habitats like Greenhouse.
	enum class Habitat : int
	{
		None = c_Invalid,
		Hogwarts,
		Trap,	// Can be spawned by a monster trap.
		Count
	};
	using HabitatBitset = std::bitset<(std::size_t)Habitat::Count>;

	// Creature Tags represent a creature type's special traits.
	// They are named like Category_Tag.  Mainly used for Fantastic Beasts.
	enum class Tag : int
	{
		None = c_Invalid,
		Bot_Blunder,		// Rarely stays in rest mode for long.
		Bot_Sidestep,		// When not attacking, circles its target.
		Colour_Rainbow,		// Cycles between rainbow colours.
		Evade_Medium,		// It gains an innate +12 bonus to evasion.
		Evade_High,			// It gains an innate +25 bonus to evasion.
		Faint_Disappear,	// When defeated, disappears instead of fainting.
		Immune_Clothes,		// No clothes, so immune to having clothes set on fire.
		Immune_Legs,		// No legs, so immune to dancing, leg-locker, etc.
		Move_Slow,			// It needs to skip a turn before moving.
		Spells_Random,		// Will be assigned some random spells near its skill level.
		Trail_Slime,		// Leaves a trail of slime clouds when it moves.
		Vision_Short,		// Can only see 3 squares instead of 8.
		Count
	};
	using TagBitset = std::bitset<(std::size_t)Tag::Count>;

	// Creature flags represent temporary conditions a creature can have.
	// Unlike statuses, flags don't have a counter or endround code.
	// Unlike tags, flags belong to a creature instance, not a type.
	enum class Flag : int
	{
		None = c_Invalid,
		MoveDelay,		// Must rest a turn before moving.  Set by Move_Slow.
		Count
	};
	using FlagBitset = std::bitset<(std::size_t)Flag::Count>;

	struct Instance
	{
		Vec3 pos = {0,0,0};
		FlagBitset flags;
		Spell::Bitset spells;
		Type type = Creature::None;
		int squad_id = c_Invalid;
		int hp = 0;
		int rest_turns = 0; // counter for healing by resting
		Item::Handle carried_item = c_Invalid;
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

	using HandleList = std::vector<Creature::Handle>;
	using HandleTempList = std::vector<Creature::Handle, Scratch<Creature::Handle>>;

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
		explicit operator int () { return index; }
		explicit operator int const () const { return index; }
		Creature::Handle & operator++ () { ++ index; return *this; }
		bool operator== (Creature::Handle rhs) const { return index == rhs.index; }
		bool operator!= (Creature::Handle rhs) const { return index != rhs.index; }

		// invalidate handle without destroying creature it points to
		void invalidate() { index = c_Invalid; }

		// Simple accessors
		bool valid () const;
		Creature::Type type () const;
		NameHash identity () const;
		bool is_generic () const;
		char const* short_name () const;
		char const* long_name () const;
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
		int num_spells () const;
		bool knows_spell (Spell::Index spell) const;
		int num_abilities () const;
		bool has_ability (Ability::Index ability) const;
		std::vector<Ability::Index> const& ability_list () const;
		bool has_habitat (Habitat habitat) const;
		bool has_tag (Tag tag) const;
		bool has_flag (Flag flag) const;
		bool ready_to_move () const;
		bool has_squad () const;
		Creature::HandleList& squad_members () const;
		Creature::Handle squad_leader () const; // by Avalon Hill
		bool has_squad_leader () const;
		bool is_squad_leader () const;
		bool has_item () const;
		Item::Handle peek_item () const;
		bool is_immune (Damage::Type damage_type) const;
		bool resists (Damage::Type damage_type) const;
		bool is_friend (Creature::Handle other_creature) const;

		// Complex accessors
		char const* colour () const;
		bool is_player () const;
		bool visible () const;
		bool finds_pos_hazardous (Vec3 pos) const;
		int vision () const;
		float miscast_rate_for_spell (Spell::Index spell) const;
		std::string status_string () const;
		Spell::TempList spells_known () const;

		// Mutators
		void take_damage (Damage::Packet const& damage_packet);
		void heal_hp (int healing);
		void move (Vec3 const & new_pos);
		void inflict_status (Status::Index status, int severity);
		void reduce_status (Status::Index status, int reduction);
		void cure_status (Status::Index status);
		void cure_all (); // heals status and hp
		void endround ();
		void rest_step ();
		void clear_rest_steps ();
		void destroy ();
		void reset_spells ();
		void learn_spell (Spell::Index spell);
		void learn_random_spells();
		void set_flag (Flag flag);
		void clear_flag (Flag flag);
		void set_squad (int new_squad_id);
		void remove_from_squad ();
		void push_item (Item::Handle item);
		Item::Handle pop_item ();
		void drop_all_items ();
		
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
		HandleItr& operator++ () { advance(); return *this; }
		Creature::Handle operator* () const { return get(); }
		Creature::Handle* operator->() { return &current; }
		explicit operator bool () const { return !finished(); }
	private:
		Creature::Handle current;
	};

	// -----------------------------------------------------------------------------------------------
	// Global interface

	void init ();
	void clear ();
	void serialize (ISerializer& s);

	bool is_valid_type (Creature::Type type);

	Creature::Handle creature_at_pos (Vec3 pos);
	bool is_anyone_at (Vec3 pos);

	Creature::Handle spawn_creature (Creature::Type type, Vec3 const & pos);
	void remove_defeated_creatures ();

	// Visible creature operations
	void update_visible_creatures ();
	void draw_creature (Creature::Handle creature_index, Draw::View const & view);
	void draw_visible_creatures (Draw::View const & view);
	Creature::HandleList const & get_visible_creatures ();
	bool has_visible_enemy ();
};
