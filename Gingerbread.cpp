#include "Creature.h"
#include "Debug.h"
#include "Gingerbread.h"
#include "House.h"
#include "Math.h"
#include "Random.h"
#include "Spell.h"
#include "VectorUtil.h"

#include <sstream>
#include <unordered_set>

namespace Gingerbread
{

// The gingerbread array stores the invariant stats for each creature type.
// (It is called gingerbread because they are like cookie cutters.)
// Note that the first entry in the array is reserved for the player.
// This is the one entry of the array that WILL change during play (as player levels up).

using TagSet = std::unordered_set<NameHash>;

static Gingerbread::Stats s_gingerbread [Creature::Count];
static Spell::Bitset s_gingerbread_spells [Creature::Count];
static TagSet s_gingerbread_tags [Creature::Count];

struct IdentityMetadata
{
	// Creature of this identity who currently exists, if any.
	Creature::Handle current_handle = Creature::None;

	// Strongest variant of this identity spawned so far.
	// They aren't allowed to repeat or regress within a game.
	float spawned_difficulty = -1.0f;
};

IdentityMetadata s_identity_metadata [(int)Identity::Count]; 

//------------------------------------------------------------------------------
// Helper function declarations

void parse_spell_string (Spell::Bitset & out_spell_bitset, std::string const & spell_string);
void parse_tag_string (std::unordered_set<NameHash> & out_tag_set, std::string const & tag_string);
void mix_gingerbread (
	Creature::Type type, Identity::Type identity, float difficulty, float probability,
	char const * short_name, char const * long_name,
	int codepoint, char const * colour, Gender gender,
	int magic_skill, int max_hp, std::string spell_string,
	std::string tag_string = "");

//------------------------------------------------------------------------------
// Global interface

void init()
{
	mix_gingerbread(Creature::Player, Identity::Player,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@', "white", Gender::Female,
		/*Magic*/ 70, /*HP*/ 90, "VM FP TA LM MW RS LC FN SP IP BT FM");

	mix_gingerbread(Creature::Neville_0, Identity::NevilleLongbottom,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f,
		"Neville", "Neville Longbottom", 'N', House::colour(House::Gryffindor), Gender::Male,
		/*Magic*/ 0, /*HP*/ 7, "VM FP",
		"Drop.Notes");

	mix_gingerbread(Creature::ColinCreevy_0, Identity::ColinCreevy,
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f,
		"Colin", "Colin Creevy", 'C', House::colour(House::Gryffindor), Gender::Male,
		/*Magic*/ 8, /*HP*/ 5, "VM MW");
	
	mix_gingerbread(Creature::SallyAnne_0, Identity::SallyAnnePerks,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f,
		"Sally-Anne", "Sally-Anne Perks", 'S', House::colour(House::Hufflepuff), Gender::Female,
		/*Magic*/ 0, /*HP*/ 3, "VM",
		"Faint.Disappear");

	mix_gingerbread(Creature::Harry_1, Identity::HarryPotter,
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		"Harry", "Harry Potter", 'H', House::colour(House::Gryffindor), Gender::Male,
		/*Magic*/ 10, /*HP*/ 12, "VM FP TA",
		"Drop.Notes");
	
	mix_gingerbread(Creature::Malfoy_1, Identity::DracoMalfoy,
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		"Malfoy", "Draco Malfoy", 'M', House::colour(House::Slytherin), Gender::Male,
		/*Magic*/ 15, /*HP*/ 10, "VM FP LM",
		"Drop.Notes");

	mix_gingerbread(Creature::Ron_2, Identity::RonWeasley,
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		"Ron", "Ron Weasley", 'R', House::colour(House::Gryffindor), Gender::Male,
		/*Magic*/ 5, /*HP*/ 16, "FP VM FM",
		"Drop.Notes");

	mix_gingerbread(Creature::Hermione_2, Identity::HermioneGranger,
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		"Hermione", "Hermione Granger", 'H', House::colour(House::Gryffindor), Gender::Female,
		/*Magic*/ 35, /*HP*/ 12, "VM MW LC", // + FI
		"Drop.Notes");

	mix_gingerbread(Creature::Crabbe_3, Identity::VincentCrabbe,
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		"Crabbe", "Vincent Crabbe", 'C', House::colour(House::Slytherin), Gender::Male,
		/*Magic*/ 20, /*HP*/ 20, "FN RS");

	mix_gingerbread(Creature::Goyle_3, Identity::GregoryGoyle,
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		"Goyle", "Gregory Goyle", 'G', House::colour(House::Slytherin), Gender::Male,
		/*Magic*/ 20, /*HP*/ 20, "VM FP TA");

	// Generic student for testing purposes
	mix_gingerbread(Creature::Hufflepuff_1, Identity::Generic,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f,
		"Hufflepuff", "First-Year Hufflepuff", 'H', House::colour(House::Hufflepuff), Gender::Male,
		/*Magic*/ 6, /*HP*/ 4, "TA VM LM");
}

void clear()
{
	for (int i = 0; i < (int)Identity::Count; ++i)
	{
		s_identity_metadata[i] = {};
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

bool has_tag(Creature::Type type, NameHash tag)
{
	if (is_valid_type(type))
	{
		return s_gingerbread_tags[type].count(tag) > 0;
	}

	DebugBreak();
	return false;
}

void reset_player_stats(House::Type house)
{
	mix_gingerbread(Creature::Player, Identity::Player,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@',
		"white", // House::colour(house),
		Gender::Female,
		/*Magic*/ (house == House::Ravenclaw) ? 15 : 10,
		/*HP*/ (house == House::Hufflepuff) ? 12 : 10,
		"");

	Creature::Handle(0).reset_spells();
	Creature::Handle(0).cure_all();
	Creature::Handle(0).update_derived_stats();
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

	float constexpr c_max_over_level = 2.0f;
	float constexpr c_max_under_level = 4.0f;

	// Probability is multiplied by this factor for each level over/under target.
	float constexpr c_over_level_factor = 0.5f;
	float constexpr c_under_level_factor = 0.75f;

	for (int type = 1; // skip player
		type < Creature::Type::Count;
		++type)
	{
		Gingerbread::Stats const& stats = s_gingerbread[type];
		Identity::Type const identity = stats.identity;

		// Exclusions
		if (stats.probability <= 0.0f ||
			Math::FloatGreater(stats.difficulty, target_difficulty + c_max_over_level) ||
			Math::FloatLess(stats.difficulty, target_difficulty - c_max_under_level))
		{
			continue;
		}

		// Identity-based considerations
		if (identity != Identity::Generic)
		{
			IdentityMetadata const& metadata = s_identity_metadata[(int)identity];

			if (metadata.current_handle != Creature::None ||
				Math::FloatLessOrEqual(stats.difficulty, metadata.spawned_difficulty))
			{
				continue;
			}
		}

		// Weight modifications
		float probability = stats.probability;

		if (Math::FloatGreater(stats.difficulty, target_difficulty))
		{
			float const difference = stats.difficulty - target_difficulty;
			probability *= pow(c_over_level_factor, difference);
		}
		else if (Math::FloatLess(stats.difficulty, target_difficulty))
		{
			float const difference = target_difficulty - stats.difficulty;
			probability *= pow(c_under_level_factor, difference);
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
	Identity::Type const identity = creature.identity();
	if (is_valid_type(type) && identity != Identity::Generic)
	{
		IdentityMetadata& metadata = s_identity_metadata[identity];

		if (metadata.current_handle != Creature::None)
		{
			// Force unspawn the old instance, and spawn the new one.
			std::cout << "Unspawning old instance of " << read(type).short_name
				<< " to spawn new instance.\n";
			metadata.current_handle.invalidate();
		}

		if (s_gingerbread[type].difficulty <= metadata.spawned_difficulty)
		{
			std::cout << "Warning: Spawning " << read(type).short_name
				<< " at lower or equal difficulty than last time.\n"
				<< " - New=" << read(type).difficulty
				<< ", old=" << metadata.spawned_difficulty << "\n";
		}

		metadata.current_handle = creature;
		metadata.spawned_difficulty = read(type).difficulty;
	}
}

void release_identity(Creature::Handle creature)
{
	Identity::Type const identity = creature.identity();
	if (identity != Identity::Generic)
	{
		if (s_identity_metadata[identity].current_handle == creature)
		{
			s_identity_metadata[identity].current_handle = Creature::None;
		}
	}
}

//------------------------------------------------------------------------------
// Helper implementations

void mix_gingerbread (
	Creature::Type type, Identity::Type identity, float difficulty, float probability,
	char const * short_name, char const * long_name,
	int codepoint, char const * colour, Gender gender,
	int magic_skill, int max_hp, std::string spell_string,
	std::string tag_string)
{
	s_gingerbread[type] = { identity, difficulty, probability,
		short_name, long_name, colour, codepoint, magic_skill, max_hp, gender };
	parse_spell_string(s_gingerbread_spells[type], spell_string);
	parse_tag_string(s_gingerbread_tags[type], tag_string);

	//if (s_gingerbread_tags[type].size() > 0)
	//{
	//	std::cout << short_name << " has " << s_gingerbread_tags[type].size() << " tags.\n";
	//}
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

void parse_tag_string (std::unordered_set<NameHash> & out_tag_set, std::string const & tag_string)
{
	out_tag_set.clear();

	if (!tag_string.empty())
	{
		std::stringstream ss(tag_string);
		std::string token;

		while (ss >> token)
		{
			//std::cout << "Adding tag: " << token << "\n";
			out_tag_set.insert(NameHash(token.c_str()));
		}
	}
}

} // namespace Creature
