#include "Gingerbread.h"

#include "Ability.h"
#include "Colour.h"
#include "Creature.h"
#include "Debug.h"
#include "House.h"
#include "Item.h"
#include "Math.h"
#include "MapUtil.h"
#include "Random.h"
#include "Serialize.h"
#include "Spell.h"
#include "VectorUtil.h"

#include <format>
#include <sstream>
#include <unordered_set>

namespace Gingerbread
{

// The gingerbread array stores the invariant stats for each creature type.
// (It is called gingerbread because they are like cookie cutters.)
// Note that the first entry in the array is reserved for the player.
// This is the one entry of the array that WILL change during play (as player levels up).

Gingerbread::Stats s_gingerbread [Creature::Count];
Spell::Bitset s_gingerbread_spells [Creature::Count];
Creature::TagBitset s_gingerbread_tags [Creature::Count];
Ragged<Ability::Index> s_gingerbread_abilities;

// List of items the creature will drop, and probability of each.
// Conventionally the weights should sum to 100, including an entry for Item::None.
std::vector<Item::Type> s_item_drops [Creature::Count];
std::vector<int> s_item_weights [Creature::Count];

// Data about how this identity has been used in the current game.
struct IdentityMetadata
{
	// Creature of this identity who currently exists, if any.
	Creature::Handle current_handle = Creature::None;

	// Strongest variant of this identity spawned so far.
	// They aren't allowed to repeat or regress within a game.
	float spawned_difficulty = -1.0f;

	void serialize(ISerializer& s);
};

struct IdentityData
{
	// The usual features of a character's identity.
	// These features may be overridden for a specific Type.
	char const* short_name = nullptr;
	char const* long_name = nullptr;
	char const* colour = nullptr;
	int codepoint = 'X';
	Gender gender = Gender::Male;
};
std::unordered_map<NameHash, IdentityData> s_identities;
std::unordered_map<NameHash, IdentityMetadata> s_metadata;

//------------------------------------------------------------------------------
// Helper function declarations

void register_identity (char const* short_name, char const* long_name,
	char const* colour, Gender gender);

void parse_spell_string (Spell::Bitset & out_spell_bitset, std::string const & spell_string);
void parse_tags (Creature::TagBitset & out_tags,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tag_list);
void mix_gingerbread(
	Creature::Type type, NameHash identity, float difficulty, float probability,
	char const * short_name, char const * long_name,
	int codepoint, char const * colour, Gender gender,
	int magic_skill, int max_hp, std::string spell_string,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tags,
	std::vector<Ability::Index>&& abilities,
	std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights);
void mix_from_identity(
	Creature::Type type, NameHash identity, float difficulty, float probability,
	int magic_skill, int max_hp, std::string spell_string,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tags,
	std::vector<Ability::Index>&& abilities,
	std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights);

//------------------------------------------------------------------------------
// Global interface

void init()
{
	using Tag = Creature::Tag;

	// Allocate memory.
	s_gingerbread_abilities.resize(Creature::Count);

	// Identities, alphabetic by short name:

	register_identity("Cedric", "Cedric Diggory", House::colour(House::Hufflepuff), Gender::Male);
	register_identity("Colin", "Colin Creevy", House::colour(House::Gryffindor), Gender::Male);
	register_identity("Crabbe", "Vincent Crabbe", House::colour(House::Slytherin), Gender::Male);
	register_identity("Fleur", "Fleur Delacour", cstr_Sky, Gender::Female);
	register_identity("Ginny", "Ginny Weasley", House::colour(House::Gryffindor), Gender::Female);
	register_identity("Goyle", "Gregory Goyle", House::colour(House::Slytherin), Gender::Male);
	register_identity("Harry", "Harry Potter", House::colour(House::Gryffindor), Gender::Male);
	register_identity("Hermione", "Hermione Granger", House::colour(House::Gryffindor), Gender::Female);
	register_identity("Krum", "Victor Krum", cstr_Flame, Gender::Male);
	register_identity("Luna", "Luna Lovegood", House::colour(House::Ravenclaw), Gender::Female);
	register_identity("Malfoy", "Draco Malfoy", House::colour(House::Slytherin), Gender::Male);
	register_identity("Neville", "Neville Longbottom", House::colour(House::Gryffindor), Gender::Male);
	register_identity("Ron", "Ron Weasley", House::colour(House::Gryffindor), Gender::Male);
	register_identity("Sally-Anne", "Sally-Anne Perks", House::colour(House::Hufflepuff), Gender::Female);

	// Creature Types, in enum order:

	mix_gingerbread(Creature::Player, c_IdentityGeneric,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@', cstr_White, Gender::Female,
		/*Magic*/ 70, /*HP*/ 90, "", {}, {},
		{}, {});

	mix_from_identity(Creature::Neville_0, "Neville",
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f,
		/*Magic*/ 0, /*HP*/ 7, "VM FP", {}, {},
		{Item::None, Item::Notes}, {50, 50});

	mix_from_identity(Creature::ColinCreevy_0, "Colin",
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f,
		/*Magic*/ 8, /*HP*/ 5, "VM MW", {}, {},
		{Item::BBBean}, {100});
	
	mix_from_identity(Creature::SallyAnne_0, "Sally-Anne",
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f,
		/*Magic*/ 0, /*HP*/ 3, "VM", {Tag::Faint_Disappear}, {},
		{}, {});

	mix_from_identity(Creature::Harry_1, "Harry",
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		/*Magic*/ 10, /*HP*/ 12, "VM FP TA", {}, {},
		{Item::Notes, Item::PotionItem}, {60, 40});
	
	mix_from_identity(Creature::Malfoy_1, "Malfoy",
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		/*Magic*/ 15, /*HP*/ 10, "VM FP LM", {}, {},
		{Item::Notes}, {100});

	mix_from_identity(Creature::Ron_2, "Ron",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		/*Magic*/ 5, /*HP*/ 16, "FP VM FM", {}, {},
		{Item::Notes}, {100});

	mix_from_identity(Creature::Hermione_2, "Hermione",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		/*Magic*/ 35, /*HP*/ 12, "VM MW LC AL FI", {}, {},
		{Item::Notes}, {100});

	mix_from_identity(Creature::Crabbe_3, "Crabbe",
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		/*Magic*/ 15, /*HP*/ 20, "FN RS", {}, {},
		{Item::Notes, Item::PotionItem, Item::None}, {10,10,80});

	mix_from_identity(Creature::Goyle_3, "Goyle",
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		/*Magic*/ 15, /*HP*/ 20, "VM FP TA", {}, {},
		{Item::Notes, Item::PotionItem, Item::None}, {10,10,80});

	mix_from_identity(Creature::Harry_4, "Harry",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f,
		/*Magic*/ 45, /*HP*/ 18, "FP TA SP IP AC", {}, {},
		{Item::Notes, Item::PotionItem}, {60, 40});

	mix_from_identity(Creature::Cedric_4, "Cedric",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f,
		/*Magic*/ 50, /*HP*/ 18, "SP RS", {}, {}, // PT, Lapifors?
		{Item::Notes}, {100});

	mix_from_identity(Creature::Fleur_4, "Fleur",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f,
		/*Magic*/ 70, /*HP*/ 16, "MW LM FP FI", {}, {}, // PT, Sleepiness?
		{Item::Notes, Item::PotionItem}, {60,40});

	mix_from_identity(Creature::Krum_5, "Krum",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f,
		/*Magic*/ 50, /*HP*/ 20, "SP FN", {}, {},
		{Item::PotionItem, Item::None}, {50,50});

	mix_from_identity(Creature::Neville_5, "Neville",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f,
		/*Magic*/ 50, /*HP*/ 20, "SP IP LM", {}, {}, // PT
		{Item::Notes}, {100});

	mix_from_identity(Creature::Ginny_5, "Ginny",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f,
		/*Magic*/ 60, /*HP*/ 18, "SP BT", {}, {}, // PT
		{Item::Notes}, {100});

	mix_from_identity(Creature::Luna_5, "Luna",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f,
		/*Magic*/ 50, /*HP*/ 17, "SP MW FM TA", {}, {}, // PT
		{Item::Notes}, {100});

	// Generic students:

	mix_gingerbread(Creature::Hufflepuff_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f,
		"Hufflepuff", "First-Year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male,
		/*Magic*/ 6, /*HP*/ 4, "TA VM LM", {}, {},
		{}, {});

	mix_gingerbread(Creature::Ravenclaw_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f,
		"Ravenclaw", "First-Year Ravenclaw", 'R', House::colour(House::Ravenclaw), Gender::Female,
		/*Magic*/ 8, /*HP*/ 3, "MW VM RS", {}, {},
		{}, {});

	mix_gingerbread(Creature::Slytherin_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f,
		"Slytherin", "First-Year Slytherin", 'S', House::colour(House::Slytherin), Gender::Female,
		/*Magic*/ 6, /*HP*/ 4, "FN FM LM", {}, {},
		{}, {});

	// Fantastic Beasts and Where To Find Them:

	mix_gingerbread(Creature::Gnome, c_IdentityGeneric,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f,
		"gnome", "garden gnome", 'g', cstr_LighterOrange, Gender::Neuter,
		/*Magic*/ 0, /*HP*/ 2, "", {}, {Ability::StealBean, Ability::EatBean},
		{}, {});

	mix_gingerbread(Creature::Streeler, c_IdentityGeneric,
		/*Difficulty*/ 0.8f, /*Probability*/ 1.0f,
		"streeler", "streeler", 's', cstr_Red, Gender::Neuter,
		/*Magic*/ 0, /*HP*/ 4, "",
		{Tag::Bot_Blunder, Tag::Colour_Rainbow, Tag::Move_Slow, Tag::Trail_Slime, Tag::Vision_Short},
		{Ability::Headbutt},
		{}, {});

	mix_gingerbread(Creature::FireCrab, c_IdentityGeneric,
		/*Difficulty*/ 1.5f, /*Probability*/ 1.0f,
		"fire crab", "fire crab", 'c', cstr_Flame, Gender::Neuter,
		/*Magic*/ 0, /*HP*/ 5, "", {Tag::Bot_Sidestep}, {Ability::ShootFire},
		{}, {});

	// Alternate universe characters!

	// Currently same as normal Harry, but 2 more hp.  Think about how to make it unique.
	// Technically his notes shouldn't be in his own handwriting, should they?
	// Possible items: monogrammed socks, magic robes, self-stirring cauldron stick, magic jar...
	mix_gingerbread(Creature::HarryTheHufflepuff_1, "Harry",
		/*Difficulty*/ 1.2f, /*Probability*/ 0.1f,
		"Harry", "Harry the Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male,
		/*Magic*/ 10, /*HP*/ 14, "VM FP TA", {}, {},
		{Item::Notes, Item::PotionItem}, {70, 30});
}

void clear()
{
	s_metadata.clear();
}

void IdentityMetadata::serialize(ISerializer& s)
{
	s.srz_creature_handle(current_handle);
	s.srz_float(spawned_difficulty);
}

void serialize(ISerializer& s)
{
	// First, need to serialize the player's mutable stats.
	Gingerbread::Stats& player_stats = s_gingerbread[0];
	s.srz_int(player_stats.skill_magic);
	s.srz_int(player_stats.max_hp);

	// Then serialize the identity metadata.
	// It will be easiest to do this one manually.
	if (s.is_load())
	{
		int count;
		s.srz_int(count);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Load Gingerbread::s_metadata, size={}", count);
		}
		s_metadata.clear();
		s_metadata.reserve(count);
		for (int i = 0; i < count; ++i)
		{
			NameHash name_hash("");
			IdentityMetadata metadata;
			s.srz_name_hash(name_hash);
			metadata.serialize(s);
			s_metadata[name_hash] = metadata;
		}
	}
	else
	{
		int count = Util::Size(s_metadata);
		s.srz_int(count);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Save Gingerbread::s_metadata, size={}", count);
		}
		for (auto& pair : s_metadata)
		{
			NameHash name_hash = pair.first;
			s.srz_name_hash(name_hash);
			pair.second.serialize(s);
		}
	}
}

bool is_valid_type (Creature::Type type)
{
	return type > Creature::None && type < Creature::Count;
}

Gingerbread::Stats const & read (Creature::Type type)
{
	if (is_valid_type(type))
	{
		return s_gingerbread[type];
	}

	DebugBreak();
	return s_gingerbread[0];
}

Spell::Bitset const& read_spells(Creature::Type type)
{
	if (is_valid_type(type))
	{
		return s_gingerbread_spells[type];
	}

	DebugBreak();
	return s_gingerbread_spells[0];
}

std::vector<Ability::Index> const& Gingerbread::read_abilities(Creature::Type type)
{
	if (is_valid_type(type))
	{
		return s_gingerbread_abilities[type];
	}

	DebugBreak();
	return s_gingerbread_abilities[0];
}

std::string short_name (Creature::Type type)
{
	if (is_valid_type(type))
	{
		return s_gingerbread[type].short_name;
	}
	return "no one";
}

std::string long_name (Creature::Type type)
{
	if (is_valid_type(type))
	{
		return s_gingerbread[type].long_name;
	}
	return "no one";
}

bool has_tag(Creature::Type type, Creature::Tag tag)
{
	if (is_valid_type(type))
	{
		return s_gingerbread_tags[type].test((size_t)tag);
	}

	DebugBreak();
	return false;
}

Item::Type random_item_drop(Creature::Type type)
{
	if (!is_valid_type(type) ||
		s_item_weights[type].empty())
	{
		return Item::None;
	}

	int const r = Random::weighted_index(s_item_weights[type]);
	return s_item_drops[type].at(r);
}

Item::Handle make_item_for_creature(Creature::Type type)
{
	Item::Type item_type = random_item_drop(type);
	if (item_type != Item::None)
	{
		switch (item_type)
		{
			case Item::BBBean:
				return Item::make_bbb();
			case Item::Notes:
				return Item::make_notes(type);
			case Item::PotionItem:
				return Item::make_potion_by_level(Gingerbread::read(type).difficulty);
		}
	}

	return c_Invalid;
}

void reset_player_stats(House::Type house)
{
	edit_player_stats().skill_magic = (house == House::Ravenclaw) ? 15 : 10;
	edit_player_stats().max_hp = (house == House::Hufflepuff) ? 12 : 10;

	Creature::Handle player_handle(0);
	if (!player_handle.valid())
	{
		// Create player instance and put it... somewhere.
		Creature::spawn_creature(Creature::Type::Player,{0,0,0});
	}
	else
	{
		Creature::Handle(0).reset_spells();
		Creature::Handle(0).cure_all();
		Creature::Handle(0).update_derived_stats();
	}
}

Gingerbread::Stats& edit_player_stats()
{
	return s_gingerbread[Creature::Player];
}

Creature::Type find_type_to_spawn (float target_difficulty)
{
	std::vector<Creature::Type> options;
	std::vector<float> weights;
	options.reserve(Creature::Count);
	weights.reserve(Creature::Count);

	float constexpr c_MaxOverLevel = 2.0f;
	float constexpr c_MaxUnderLevel = 4.0f;

	// Probability is multiplied by this factor for each level over/under target.
	float constexpr c_OverLevelFactor = 0.5f;
	float constexpr c_UnderLevelFactor = 0.75f;

	for (int type = 1; // skip player
		type < Creature::Type::Count;
		++type)
	{
		Gingerbread::Stats const& stats = s_gingerbread[type];
		NameHash const identity = stats.identity;

		// Exclusions
		if (stats.probability <= 0.0f ||
			Math::FloatGreater(stats.difficulty, target_difficulty + c_MaxOverLevel) ||
			Math::FloatLess(stats.difficulty, target_difficulty - c_MaxUnderLevel))
		{
			continue;
		}

		// Identity-based considerations.
		// Can't have two of the same person running around at the same time,
		// and must increase difficulty by at least 1.0 when respawning the same character.
		if (identity != c_IdentityGeneric)
		{
			IdentityMetadata const& metadata = s_metadata[identity];

			if (metadata.current_handle != Creature::None ||
				Math::FloatLess(stats.difficulty, metadata.spawned_difficulty + 1.0f))
			{
				continue;
			}
		}

		// Weight modifications
		float probability = stats.probability;

		if (Math::FloatGreater(stats.difficulty, target_difficulty))
		{
			float const difference = stats.difficulty - target_difficulty;
			probability *= pow(c_OverLevelFactor, difference);
		}
		else if (Math::FloatLess(stats.difficulty, target_difficulty))
		{
			float const difference = target_difficulty - stats.difficulty;
			probability *= pow(c_UnderLevelFactor, difference);
		}

		if (probability > 0.0f)
		{
			options.push_back((Creature::Type)type);
			weights.push_back(probability);
		}
	}

	if (Util::Size(options) > 0)
	{
		int const choice = Random::weighted_index(weights);
		assert(Util::IsValidIndex(options, choice));
		return options.at(choice);
	}
	else
	{
		return Creature::None;
	}
}

void claim_identity(Creature::Handle creature)
{
	Creature::Type const type = creature.type();
	NameHash const identity = creature.identity();
	if (is_valid_type(type) && identity != c_IdentityGeneric)
	{
		IdentityMetadata& metadata = s_metadata[identity];

		if (metadata.current_handle != Creature::None)
		{
			// Force unspawn the old instance, and spawn the new one.
			std::cout << std::format("Unspawning old instance of {} to spawn new instance.\n",
				read(type).short_name);
			metadata.current_handle.destroy();
		}

		if (read(type).difficulty <= metadata.spawned_difficulty)
		{
			std::cout << std::format(
				"Spawning {} at lower or equal difficulty than last time; New={}, Old={}\n",
				read(type).short_name, read(type).difficulty, metadata.spawned_difficulty);
		}

		metadata.current_handle = creature;
		metadata.spawned_difficulty = read(type).difficulty;
	}
}

void release_identity(Creature::Handle creature)
{
	NameHash const identity = creature.identity();
	if (identity != c_IdentityGeneric)
	{
		IdentityMetadata* metadata = Util::Find(s_metadata, identity);
		if (metadata && metadata->current_handle == creature)
		{
			metadata->current_handle = Creature::None;
		}
	}
}

//------------------------------------------------------------------------------
// Helper implementations

void register_identity (char const* short_name, char const* long_name,
	char const* colour, Gender gender)
{
	s_identities[short_name] =
	{
		short_name,
		long_name,
		colour,
		short_name[0],
		gender
	};

	s_metadata[short_name] = IdentityMetadata{};
}

void mix_gingerbread(
	Creature::Type type, NameHash identity, float difficulty, float probability,
	char const * short_name, char const * long_name,
	int codepoint, char const * colour, Gender gender,
	int magic_skill, int max_hp, std::string spell_string,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tags,
	std::vector<Ability::Index>&& abilities,
	std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights)
{
	s_gingerbread[type] = { identity, difficulty, probability,
		short_name, long_name, colour, codepoint, magic_skill, max_hp, gender };
	parse_spell_string(s_gingerbread_spells[type], spell_string);
	parse_tags(s_gingerbread_tags[type], tags);
	s_gingerbread_abilities[type] = abilities;
	s_item_drops[type] = std::move(item_drops);
	s_item_weights[type] = std::move(item_weights);
	assert(Util::Size(s_item_drops[type]) == Util::Size(s_item_weights[type]));
}

void mix_from_identity(
	Creature::Type type, NameHash identity, float difficulty, float probability,
	int magic_skill, int max_hp, std::string spell_string,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tags,
	std::vector<Ability::Index>&& abilities,
	std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights)
{
	IdentityData const& data = s_identities.at(identity);
	mix_gingerbread(type, identity, difficulty, probability,
		data.short_name, data.long_name, data.codepoint, data.colour, data.gender,
		magic_skill, max_hp, spell_string, tags, std::move(abilities),
		std::move(item_drops), std::move(item_weights));
}

void parse_spell_string (Spell::Bitset & out_spell_bitset, std::string const & spell_string)
{
	out_spell_bitset.reset();
	std::stringstream ss(spell_string);

	std::string token;
	while (ss >> token)
	{
		Spell::Index spell = Spell::get_index_by_abbrev(token);
		assert(spell != Spell::None);
		out_spell_bitset.set(spell, true);
	}
}

void parse_tags (Creature::TagBitset & out_tags,
	std::vector<Creature::Tag,Scratch<Creature::Tag>> const& tag_list)
{
	out_tags.set(false);

	for (Creature::Tag tag : tag_list)
	{
		out_tags.set((std::size_t)tag, true);
	}
}

} // namespace Creature
