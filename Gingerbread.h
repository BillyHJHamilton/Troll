#pragma once

#include "NameHash.h"
#include "Types.h"
#include "Spell.h" // for bitset

namespace Identity
{
	enum Type : int
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
}

namespace Gingerbread
{
	struct Stats
	{
		Identity::Type identity = (Identity::Type)c_invalid;
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

	void init();
	void clear();

	bool is_valid_type(Creature::Type type);
	Stats const& read(Creature::Type type);
	Spell::Bitset const& read_spells(Creature::Type type);
	const char* short_name(Creature::Type type);
	const char* long_name(Creature::Type type);
	bool has_tag(Creature::Type type, NameHash tag);

	void reset_player_stats(House::Type house);
	Stats& edit_player_stats();
	Creature::Type find_type_to_spawn (float target_difficulty);

	void claim_identity(Creature::Handle creature);
	void release_identity(Creature::Handle creature);
}
