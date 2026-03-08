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
Grid<float> s_resistances; // (creature type, damage type)

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

// Convenience class for initializing creature stats.
struct Builder
{
	Builder(Creature::Type type, NameHash identity,
		float difficulty, float probability, int max_hp);
	Builder(Creature::Type type, NameHash identity,
		float difficulty, float probability, int max_hp,
		char const* short_name, char const* long_name,
		int codepoint, char const* colour, Gender gender);

	Builder& magic(int skill, std::string spell_string);
	Builder& tags(Creature::Tag new_tag);
	Builder& abil(std::vector<Ability::Index>&& abilities);
	Builder& item(std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights);
	Builder& resist(Damage::Type type);
	Builder& vuln(Damage::Type type);
	Builder& immune(Damage::Type type);
	
	template<class ... Packed>
	Builder& tags(Creature::Tag tag, Packed... args)
	{
		tags(tag);
		return tags(args...);
	}

	template<class ... Packed>
	Builder& resist(Damage::Type type, Packed... args)
	{
		resist(type);
		return resist(args...);
	}

	template<class ... Packed>
	Builder& vuln(Damage::Type type, Packed... args)
	{
		vuln(type);
		return vuln(args...);
	}

	template<class ... Packed>
	Builder& immune(Damage::Type type, Packed... args)
	{
		immune(type);
		return immune(args...);
	}

protected:
	Creature::Type m_type;
};

//------------------------------------------------------------------------------
// Helper function declarations

void register_identity (char const* short_name, char const* long_name,
	char const* colour, Gender gender);

void parse_spell_string (Spell::Bitset & out_spell_bitset, std::string const & spell_string);

//------------------------------------------------------------------------------
// Global interface

void init()
{
	using Tag = Creature::Tag;

	// Allocate memory.
	s_gingerbread_abilities.resize(Creature::Count);
	s_resistances = Grid<float>(Creature::Count, Damage::Type::Count, 1.0f);

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

	Builder(Creature::Player, c_IdentityGeneric,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f, /*HP*/ 10,
		"You", "You", '@', cstr_White, Gender::Female);

	Builder(Creature::Neville_0, "Neville",
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f, /*HP*/ 7)
		.magic(0, "VM FP")
		.item({Item::None, Item::Notes}, {50, 50});

	// TODO: Camera ability (or item, which player can also use).
	// It should blind you (reduce LOS) and produce some smoke, too.
	Builder(Creature::ColinCreevy_0, "Colin",
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f, /*HP*/ 5)
		.magic(8, "VM MW")
		.item({Item::BBBean}, {100});
	
	Builder(Creature::SallyAnne_0, "Sally-Anne",
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f, /*HP*/ 3)
		.magic(0, "VM")
		.tags(Tag::Faint_Disappear);

	Builder(Creature::Harry_1, "Harry", 
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f, /*HP*/ 12)
		.magic(10, "VM FP LM")
		.item({Item::Notes, Item::PotionItem}, {60, 40});
	
	Builder(Creature::Malfoy_1, "Malfoy",
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f, /*HP*/ 10)
		.magic(15, "VM FP TA")
		.item({Item::Notes}, {100});

	Builder(Creature::Ron_2, "Ron",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f, /*HP*/ 14)
		.magic(5, "FP VM FM")
		.item({Item::Notes}, {100});

	Builder(Creature::Hermione_2, "Hermione",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f, /*HP*/ 12)
		.magic(35, "VM MW LC AL FI")
		.item({Item::Notes}, {100});

	Builder(Creature::Crabbe_3, "Crabbe",
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(15, "FN RS")
		.item({Item::Notes, Item::PotionItem, Item::None}, {10,10,80});

	Builder(Creature::Goyle_3, "Goyle",
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(15, "VM FP LM")
		.item({Item::Notes, Item::PotionItem, Item::None}, {10,10,80});

	Builder(Creature::Harry_4, "Harry",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(45, "FP TA SP IP AC")
		.item({Item::Notes, Item::PotionItem}, {60, 40});

	Builder(Creature::Cedric_4, "Cedric",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(50, "SP RS") // PT, Lapifors?
		.item({Item::Notes}, {100});

	Builder(Creature::Fleur_4, "Fleur",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 16)
		.magic(70, "MW LM FP FI") // PT, Sleepiness?
		.item({Item::Notes, Item::PotionItem}, {60,40});

	Builder(Creature::Krum_5, "Krum",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(50, "SP FN")
		.item({Item::PotionItem, Item::None}, {50,50});

	Builder(Creature::Neville_5, "Neville",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(50, "SP IP LM") // PT
		.item({Item::Notes}, {100});

	Builder(Creature::Ginny_5, "Ginny",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(60, "SP BT") // PT
		.item({Item::Notes}, {100});

	Builder(Creature::Luna_5, "Luna",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 17)
		.magic(50, "SP MW FM TA") // PT
		.item({Item::Notes}, {100});

	// Generic students:

	Builder(Creature::Hufflepuff_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f, /*HP*/ 4,
		"Hufflepuff", "First-Year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male)
		.magic(6, "TA VM LM");

	Builder(Creature::Ravenclaw_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f, /*HP*/ 3,
		"Ravenclaw", "First-Year Ravenclaw", 'R', House::colour(House::Ravenclaw), Gender::Female)
		.magic(8, "MW VM RS");

	Builder(Creature::Slytherin_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f, /*HP*/ 4,
		"Slytherin", "First-Year Slytherin", 'S', House::colour(House::Slytherin), Gender::Female)
		.magic(6, "FN FM LM");

	// Fantastic Beasts and Where To Find Them:

	Builder(Creature::Gnome, c_IdentityGeneric,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f, /*HP*/ 2,
		"gnome", "garden gnome", 'g', cstr_LighterOrange, Gender::Neuter)
		.tags(Tag::Immune_Clothes)
		.abil({Ability::StealBean, Ability::EatBean});

	Builder(Creature::Streeler, c_IdentityGeneric,
		/*Difficulty*/ 0.8f, /*Probability*/ 1.0f, /*HP*/ 4,
		"streeler", "streeler", 's', cstr_Red, Gender::Neuter)
		.tags(Tag::Bot_Blunder, Tag::Colour_Rainbow, Tag::Immune_Clothes, Tag::Immune_Legs,
			Tag::Move_Slow, Tag::Trail_Slime, Tag::Vision_Short)
		.immune(Damage::Acid)
		.abil({Ability::Headbutt});

	Builder(Creature::FireCrab, c_IdentityGeneric,
		/*Difficulty*/ 1.5f, /*Probability*/ 1.0f, /*HP*/ 5,
		"fire crab", "fire crab", 'c', cstr_Flame, Gender::Neuter)
		.tags(Tag::Bot_Sidestep, Tag::Immune_Clothes)
		.resist(Damage::Fire)
		.abil({Ability::ShootFire});

	// Alternate universe characters:

	// Similar to normal Harry, but 2 more hp and moves slowly.
	// Technically his notes shouldn't be in his own handwriting, should they?
	// Possible items: monogrammed socks, magic robes, self-stirring cauldron stick, magic jar...
	Builder(Creature::HarryTheHufflepuff_1, "Harry",
		/*Difficulty*/ 1.2f, /*Probability*/ 0.1f, /*HP*/ 14,
		"Harry", "Harry the Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male)
		.magic(10, "VM FP LM")
		.tags(Creature::Tag::Move_Slow)
		.item({Item::Notes, Item::PotionItem}, {70, 30});
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

float Gingerbread::read_resistance(Creature::Type type, Damage::Type damage_type)
{
	if (is_valid_type(type))
	{
		return s_resistances.read(type, damage_type);
	}

	DebugBreak();
	return 1.0f;
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

Builder::Builder(Creature::Type type, NameHash identity,
	float difficulty, float probability, int max_hp,
	char const* short_name, char const* long_name,
	int codepoint, char const* colour, Gender gender) :
	m_type(type)
{
	s_gingerbread[m_type].identity = identity;
	s_gingerbread[m_type].difficulty = difficulty;
	s_gingerbread[m_type].probability = probability;
	s_gingerbread[m_type].max_hp = max_hp;
	s_gingerbread[m_type].short_name = short_name;
	s_gingerbread[m_type].long_name = long_name;
	s_gingerbread[m_type].codepoint = codepoint;
	s_gingerbread[m_type].colour = colour;
	s_gingerbread[m_type].gender = gender;
}

Builder::Builder(Creature::Type type, NameHash identity,
	float difficulty, float probability, int max_hp) :
	m_type(type)
{
	IdentityData const& data = s_identities.at(identity);
		
	s_gingerbread[m_type].identity = identity;
	s_gingerbread[m_type].difficulty = difficulty;
	s_gingerbread[m_type].probability = probability;
	s_gingerbread[m_type].max_hp = max_hp;
	s_gingerbread[m_type].short_name = data.short_name;
	s_gingerbread[m_type].long_name = data.long_name;
	s_gingerbread[m_type].codepoint = data.codepoint;
	s_gingerbread[m_type].colour = data.colour;
	s_gingerbread[m_type].gender = data.gender;
}

Builder& Builder::magic(int skill_magic, std::string spell_string)
{
	s_gingerbread[m_type].skill_magic = skill_magic;
	parse_spell_string(s_gingerbread_spells[m_type], spell_string);
	return *this;
}

Builder& Builder::tags(Creature::Tag tag)
{
	s_gingerbread_tags[m_type].set((size_t)tag, true);
	return *this;
}

Builder& Builder::abil(std::vector<Ability::Index>&& abilities)
{
	s_gingerbread_abilities[m_type] = abilities;
	return *this;
}

Builder& Builder::item(std::vector<Item::Type>&& item_drops, std::vector<int>&& item_weights)
{
	s_item_drops[m_type] = std::move(item_drops);
	s_item_weights[m_type] = std::move(item_weights);
	return *this;
}

Builder& Builder::resist(Damage::Type type)
{
	s_resistances.edit(m_type, (int)type) = 0.5f;
	return *this;
}

Builder& Builder::vuln(Damage::Type type)
{
	s_resistances.edit(m_type, (int)type) = 2.0f;
	return *this;
}

Builder& Builder::immune(Damage::Type type)
{
	s_resistances.edit(m_type, (int)type) = 0.0f;
	return *this;
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

} // namespace Creature
