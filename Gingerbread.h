#pragma once

#include "Types.h"

#include "Creature.h" // for Creature::Tag::None
#include "NameHash.h"
#include "Spawn.h"
#include "Spell.h" // for bitset

NameHash constexpr c_IdentityGeneric {"Generic"};

namespace Gingerbread
{
	struct Stats
	{
		NameHash identity = c_IdentityGeneric;
		float difficulty = 0.0f;
		float probability = 1.0f;
		char const * short_name = nullptr;
		char const * long_name = nullptr;
		char const * colour = nullptr;
		int codepoint = 0; // letter to display
		int skill_magic = 0;
		int max_hp = 0;
		Gender gender = (Gender)0;
	};

	void init();
	void clear();
	void serialize(ISerializer& s);

	Stats const& read(Creature::Type type);
	Spell::Bitset const& read_spells(Creature::Type type);
	float read_resistance(Creature::Type type, Damage::Type damage_type);
	std::vector<Ability::Index> const& read_abilities(Creature::Type type);
	std::string short_name(Creature::Type type);
	std::string long_name(Creature::Type type);
	bool has_habitat(Creature::Type type, Creature::Habitat habitat);
	bool has_tag(Creature::Type type, Creature::Tag tag);
	void provide_items(Creature::Handle creature);

	void reset_player_stats(House::Type house);
	Stats& edit_player_stats();

	bool can_spawn_identity (Creature::Type type, float target_difficulty);
	void find_spawn_options (float target_difficulty, Spawn::OptionTempList& out_list,
		FloatTempList& out_weights, Creature::Habitat habitat = Creature::Habitat::None);

	//Creature::Type find_type_to_spawn (float target_difficulty);

	void claim_identity(Creature::Handle creature);
	void release_identity(Creature::Handle creature);

	// Finds current creature with same identity, if any (not necessarily same type).
	// Returns invalid handle if generic or not currently spawned.
	Creature::Handle find_incarnation(Creature::Type type_for_identity);
}
