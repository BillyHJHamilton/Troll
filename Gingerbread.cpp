#include "Gingerbread.h"

#include "Ability.h"
#include "Colour.h"
#include "Creature.h"
#include "Debug.h"
#include "House.h"
#include "Item.h"
#include "Loot.h"
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
Creature::HabitatBitset s_gingerbread_habitats [Creature::Count];
Creature::TagBitset s_gingerbread_tags [Creature::Count];
Ragged<Ability::Index> s_gingerbread_abilities;
Grid<float> s_resistances; // (creature type, damage type)

// List of items the creature will carry.
// For each loot slot there is a loot list to choose the item type from,
// and a percent likelihood that this loot will be added.
std::vector<Loot::TypePercent> s_loot [Creature::Count];

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
	Builder& habitats(Creature::Habitat new_habitat);
	Builder& tags(Creature::Tag new_tag);
	Builder& abil(std::vector<Ability::Index>&& abilities);
	Builder& loot(Loot::Type loot_type); // implying 100%
	Builder& loot(Loot::Type loot_type, int percent);
	Builder& loot(std::vector<Loot::TypePercent>&& loot_list);
	Builder& resist(Damage::Type type);
	Builder& vuln(Damage::Type type);
	Builder& immune(Damage::Type type);

	template<class ... Packed>
	Builder& habitats(Creature::Habitat habitat, Packed... args)
	{
		habitats(habitat);
		return tags(args...);
	}

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
	using Habitat = Creature::Habitat;

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
	register_identity("Mary Sue", "Mary Sue", cstr_Pink, Gender::Female);
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
		.loot(Loot::Notes, 50);

	// TODO: Camera ability (or item, which player can also use).
	// It should blind you (reduce LOS) and produce some smoke, too.
	Builder(Creature::ColinCreevy_0, "Colin",
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f, /*HP*/ 5)
		.magic(8, "VM MW")
		.loot(Loot::Student_Generic);
	
	Builder(Creature::SallyAnne_0, "Sally-Anne",
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f, /*HP*/ 3)
		.magic(0, "VM")
		.tags(Tag::Faint_Disappear);

	Builder(Creature::Harry_1, "Harry", 
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f, /*HP*/ 12)
		.magic(10, "VM FP LM")
		.loot({{Loot::Notes, 60}, {Loot::Student_Generic, 60}});
	
	Builder(Creature::Malfoy_1, "Malfoy",
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f, /*HP*/ 10)
		.magic(15, "VM FP TA")
		.loot(Loot::Notes);

	Builder(Creature::Ron_2, "Ron",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f, /*HP*/ 14)
		.magic(5, "FP VM FM")
		.loot({{Loot::Notes, 60}, {Loot::Student_Generic, 60}});

	Builder(Creature::Hermione_2, "Hermione",
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f, /*HP*/ 12)
		.magic(35, "VM MW LC AL FI")
		.loot(Loot::Notes);

	// Crabbe and Goyle have 0 probability because they spawn in a squad instead.
	Builder(Creature::Crabbe_3, "Crabbe",
		/*Difficulty*/ 2.3f, /*Probability*/ 0.0f, /*HP*/ 15)
		.magic(15, "FN RS")
		.loot({{Loot::Notes, 20}, {Loot::Sweets, 60}});

	Builder(Creature::Goyle_3, "Goyle",
		/*Difficulty*/ 2.3f, /*Probability*/ 0.0f, /*HP*/ 15)
		.magic(15, "VM FP LM")
		.loot({{Loot::Notes, 20}, {Loot::Sweets, 60}});

	Builder(Creature::Harry_4, "Harry",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(45, "FP TA SP IP AC")
		.loot(Loot::Notes);

	Builder(Creature::Cedric_4, "Cedric",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(50, "SP RS") // PT, Lapifors?
		.loot(Loot::Notes);

	Builder(Creature::Fleur_4, "Fleur",
		/*Difficulty*/ 4.0f, /*Probability*/ 1.0f, /*HP*/ 16)
		.magic(70, "MW LM FP FI") // PT, Sleepiness?
		.loot({{Loot::Notes, 70}, {Loot::Potion, 60}});

	Builder(Creature::Krum_5, "Krum",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(50, "SP FN")
		.loot(Loot::Potion, 50);

	Builder(Creature::Neville_5, "Neville",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 20)
		.magic(50, "SP IP LM") // PT
		.loot(Loot::Notes);

	Builder(Creature::Ginny_5, "Ginny",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 18)
		.magic(60, "SP BT") // PT
		.loot(Loot::Notes);

	Builder(Creature::Luna_5, "Luna",
		/*Difficulty*/ 5.0f, /*Probability*/ 1.0f, /*HP*/ 17)
		.magic(50, "SP MW FM TA") // PT
		.loot(Loot::Notes);

	Builder(Creature::MarySue, "Mary Sue",
		/*Difficulty*/ 7.0f, /*Probability*/ 0.0f, /*HP*/ 30)
		.magic(80, "SP RS BT FI")
		.abil({Ability::Believe, Ability::Karate});

	// Generic students:

	// First-years

	Builder(Creature::Hufflepuff_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.2f, /*HP*/ 5,
		"Hufflepuff", "first-year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male)
		.magic(6, "TA")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Ravenclaw_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.2f, /*HP*/ 3,
		"Ravenclaw", "first-year Ravenclaw", 'R', House::colour(House::Ravenclaw), Gender::Female)
		.magic(8, "MW")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Gryffindor_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.2f, /*HP*/ 4,
		"Gryffindor", "first-year Gryffindor", 'G', House::colour(House::Gryffindor), Gender::Male)
		.magic(7, "FP")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Slytherin_1, c_IdentityGeneric,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.2f, /*HP*/ 4,
		"Slytherin", "first-year Slytherin", 'S', House::colour(House::Slytherin), Gender::Female)
		.magic(7, "FN")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	// Second-years

	Builder(Creature::Hufflepuff_2, c_IdentityGeneric,
		/*Difficulty*/ 1.8f, /*Probability*/ 0.2f, /*HP*/ 7,
		"Hufflepuff", "second-year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Female)
		.magic(12, "TA")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Ravenclaw_2, c_IdentityGeneric,
		/*Difficulty*/ 1.8f, /*Probability*/ 0.2f, /*HP*/ 5,
		"Ravenclaw", "second-year Ravenclaw", 'R', House::colour(House::Ravenclaw), Gender::Male)
		.magic(20, "MW")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Gryffindor_2, c_IdentityGeneric,
		/*Difficulty*/ 1.8f, /*Probability*/ 0.2f, /*HP*/ 6,
		"Gryffindor", "second-year Gryffindor", 'G', House::colour(House::Gryffindor), Gender::Female)
		.magic(15, "RS")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Slytherin_2, c_IdentityGeneric,
		/*Difficulty*/ 1.8f, /*Probability*/ 0.2f, /*HP*/ 6,
		"Slytherin", "second-year Slytherin", 'S', House::colour(House::Slytherin), Gender::Male)
		.magic(15, "LM")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	// Third-years

	Builder(Creature::Hufflepuff_3, c_IdentityGeneric,
		/*Difficulty*/ 2.8f, /*Probability*/ 0.2f, /*HP*/ 14,
		"Hufflepuff", "third-year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male)
		.magic(22, "TA")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Ravenclaw_3, c_IdentityGeneric,
		/*Difficulty*/ 2.8f, /*Probability*/ 0.2f, /*HP*/ 9,
		"Ravenclaw", "third-year Ravenclaw", 'R', House::colour(House::Ravenclaw), Gender::Female)
		.magic(30, "MW")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Gryffindor_3, c_IdentityGeneric,
		/*Difficulty*/ 2.8f, /*Probability*/ 0.2f, /*HP*/ 11,
		"Gryffindor", "third-year Gryffindor", 'G', House::colour(House::Gryffindor), Gender::Female)
		.magic(24, "FP")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	Builder(Creature::Slytherin_3, c_IdentityGeneric,
		/*Difficulty*/ 2.8f, /*Probability*/ 0.2f, /*HP*/ 10,
		"Slytherin", "third-year Slytherin", 'S', House::colour(House::Slytherin), Gender::Male)
		.magic(26, "FN")
		.tags(Tag::Spells_Random).loot(Loot::Student_Generic);

	// Fantastic Beasts and Where To Find Them:

	Builder(Creature::Gnome, c_IdentityGeneric,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f, /*HP*/ 2,
		"gnome", "garden gnome", 'g', cstr_LightOrange, Gender::Neuter)
		.tags(Tag::Immune_Clothes, Tag::Evade_High)
		.abil({Ability::StealBean, Ability::EatBean});

	Builder(Creature::Streeler, c_IdentityGeneric,
		/*Difficulty*/ 0.8f, /*Probability*/ 0.8f, /*HP*/ 4,
		"streeler", "streeler", 's', cstr_Red, Gender::Neuter)
		.habitats(Habitat::Trap)
		.tags(Tag::Bot_Blunder, Tag::Colour_Rainbow, Tag::Immune_Clothes, Tag::Immune_Legs,
			Tag::Move_Slow, Tag::Trail_Slime, Tag::Vision_Short)
		.immune(Damage::Acid)
		.abil({Ability::Headbutt});

	Builder(Creature::FireCrab, c_IdentityGeneric,
		/*Difficulty*/ 1.5f, /*Probability*/ 0.8f, /*HP*/ 5,
		"fire crab", "fire crab", 'c', cstr_Flame, Gender::Neuter)
		.habitats(Habitat::Trap)
		.tags(Tag::Bot_Sidestep, Tag::Immune_Clothes)
		.resist(Damage::Fire)
		.abil({Ability::ShootFire});

	Builder(Creature::BigFireCrab, c_IdentityGeneric,
		/*Difficulty*/ 2.5f, /*Probability*/ 0.2f, /*HP*/ 10,
		"big fire crab", "big fire crab", 'c', cstr_Crimson, Gender::Neuter)
		.habitats(Habitat::Trap)
		.tags(Tag::Bot_Sidestep, Tag::Immune_Clothes)
		.resist(Damage::Fire)
		.abil({Ability::ShootFire});

	Builder(Creature::Doxy, c_IdentityGeneric,
		/*Difficulty*/ 1.5f, /*Probability*/ 0.6f, /*HP*/ 4,
		"doxy", "doxy", 'd', cstr_LighterBlue, Gender::Neuter)
		.tags(Tag::Evade_High, Tag::Immune_Legs, Tag::Immune_Clothes)
		.abil({Ability::DoxyBite});
	
	Builder(Creature::Imp, c_IdentityGeneric,
		/*Difficulty*/ 2.0f, /*Probability*/ 0.5f, /*HP*/ 5,
		"imp", "imp", 'i', cstr_LighterOrange, Gender::Neuter)
		.tags(Tag::Evade_Medium, Tag::Immune_Clothes)
		.abil({Ability::TripKick, Ability::Scratch});

	// Alternate universe characters:

	// Similar to normal Harry, but 2 more hp and moves slowly.
	// Technically his notes shouldn't be in his own handwriting, should they?
	// Possible items: monogrammed socks, magic robes, self-stirring cauldron stick, magic jar...
	Builder(Creature::HarryTheHufflepuff_1, "Harry",
		/*Difficulty*/ 1.2f, /*Probability*/ 0.1f, /*HP*/ 14,
		"Harry", "Harry the Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male)
		.magic(10, "VM FP LM")
		.tags(Creature::Tag::Move_Slow)
		.loot({{Loot::Notes, 70}, {Loot::Potion, 40}});

	// Validate that we didn't miss something.
	for (int i = 0; i < Creature::Count; ++i)
	{
		assert(s_gingerbread[i].short_name != nullptr);
		assert(s_gingerbread[i].long_name != nullptr);
		assert(s_gingerbread[i].codepoint != 0);
	}
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

Gingerbread::Stats const & read (Creature::Type type)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread[type];
	}

	DebugBreak();
	return s_gingerbread[0];
}

Spell::Bitset const& read_spells(Creature::Type type)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread_spells[type];
	}

	DebugBreak();
	return s_gingerbread_spells[0];
}

float Gingerbread::read_resistance(Creature::Type type, Damage::Type damage_type)
{
	if (Creature::is_valid_type(type))
	{
		return s_resistances.read(type, damage_type);
	}

	DebugBreak();
	return 1.0f;
}

std::vector<Ability::Index> const& Gingerbread::read_abilities(Creature::Type type)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread_abilities[type];
	}

	DebugBreak();
	return s_gingerbread_abilities[0];
}

std::string short_name (Creature::Type type)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread[type].short_name;
	}
	return "no one";
}

std::string long_name (Creature::Type type)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread[type].long_name;
	}
	return "no one";
}

bool has_habitat(Creature::Type type, Creature::Habitat habitat)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread_habitats[type].test((size_t)habitat);
	}

	DebugBreak();
	return false;
}

bool has_tag(Creature::Type type, Creature::Tag tag)
{
	if (Creature::is_valid_type(type))
	{
		return s_gingerbread_tags[type].test((size_t)tag);
	}

	DebugBreak();
	return false;
}

void provide_items(Creature::Handle creature)
{
	Creature::Type const creature_type = creature.type();
	std::vector<Loot::TypePercent> const& loot_list = s_loot[creature_type];
	for (Loot::TypePercent type_percent : loot_list)
	{
		Item::Handle const item = Loot::make(type_percent, creature_type,
			Gingerbread::read(creature_type).difficulty);
		if (item.valid())
		{
			creature.push_item(item);
		}
	}
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

bool can_spawn_identity (Creature::Type type, float target_difficulty)
{
	Gingerbread::Stats const& stats = s_gingerbread[type];
	NameHash const identity = stats.identity;

	// Identity-based considerations.
	// Can't have two of the same person running around at the same time,
	// and must increase difficulty by at least 1.0 when respawning the same character.
	if (identity != c_IdentityGeneric)
	{
		IdentityMetadata const& metadata = s_metadata[identity];

		if (metadata.current_handle != Creature::None ||
			Math::FloatLess(stats.difficulty, metadata.spawned_difficulty + 1.0f))
		{
			return false;
		}
	}

	return true;
}

void find_spawn_options (float target_difficulty, Spawn::OptionTempList& out_list,
	FloatTempList& out_weights, Creature::Habitat habitat)
{
	for (int type = 1; // skip player
		type < Creature::Type::Count;
		++type)
	{
		Gingerbread::Stats const& stats = s_gingerbread[type];

		if (stats.probability <= 0.0f ||
			!Spawn::difficulty_in_range(stats.difficulty, target_difficulty) ||
			!can_spawn_identity((Creature::Type)type, target_difficulty))
		{
			continue;
		}

		if (habitat != Creature::Habitat::None &&
			!has_habitat((Creature::Type)type, habitat))
		{
			continue;
		}

		float const probability = stats.probability *
			Spawn::probability_factor(stats.difficulty, target_difficulty);

		if (probability > 0.0f)
		{
			out_list.emplace_back(Spawn::Option::Type::Creature, type);
			out_weights.push_back(probability);
		}
	}
}
/*
Creature::Type find_type_to_spawn (float target_difficulty)
{
	Spawn::OptionTempList options;
	FloatTempList weights;
	options.reserve(Creature::Count);
	weights.reserve(Creature::Count);

	find_spawn_options(target_difficulty, options, weights);

	if (Util::Size(options) > 0)
	{
		int const choice = Random::weighted_index(weights);
		assert(Util::IsValidIndex(options, choice));
		return (Creature::Type)options.at(choice).index;
	}
	else
	{
		return Creature::None;
	}
}
*/
void claim_identity(Creature::Handle creature)
{
	Creature::Type const type = creature.type();
	NameHash const identity = creature.identity();
	if (Creature::is_valid_type(type) && identity != c_IdentityGeneric)
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

Builder& Builder::habitats(Creature::Habitat habitat)
{
	s_gingerbread_habitats[m_type].set((size_t)habitat, true);
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

Builder& Builder::loot(Loot::Type loot_type)
{
	s_loot[m_type].push_back({loot_type, 100});
	return *this;
}

Builder& Builder::loot(Loot::Type loot_type, int percent)
{
	s_loot[m_type].push_back({loot_type, percent});
	return *this;
}

Builder& Builder::loot(std::vector<Loot::TypePercent>&& list)
{
	s_loot[m_type] = std::move(list);
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
