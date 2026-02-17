#pragma once

#include "NameHash.h"
#include "Types.h"
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

	bool is_valid_type(Creature::Type type);
	Stats const& read(Creature::Type type);
	Spell::Bitset const& read_spells(Creature::Type type);
	std::string short_name(Creature::Type type);
	std::string long_name(Creature::Type type);
	bool has_tag(Creature::Type type, NameHash tag);
	Item::Type random_item_drop(Creature::Type type);

	void reset_player_stats(House::Type house);
	Stats& edit_player_stats();
	Creature::Type find_type_to_spawn (float target_difficulty);

	void claim_identity(Creature::Handle creature);
	void release_identity(Creature::Handle creature);
}
