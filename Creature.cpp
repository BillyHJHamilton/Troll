#include "Creature.h"

#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "Bot.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Math.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"
#include "VectorUtil.h"
#include "World.h"

namespace Creature
{

//-------------------------------------------------------------------------------------------------
// Metadata about creature identities.

struct IdentityMetadata
{
	// Creature of this identity who currently exists, if any.
	Creature::Handle current_handle = Creature::None;

	// Strongest variant of this identity spawned so far.
	// They aren't allowed to repeat or regress within a game.
	float spawned_difficulty = -1.0f;
};

IdentityMetadata s_identity_metadata [(int)Identity::Count]; 

//-------------------------------------------------------------------------------------------------

// The gingerbread array stores the invariant stats for each creature type.
// (It is called gingerbread because they are like cookie cutters.)
// Note that the first entry in the array is reserved for the player.
// This is the one entry of the array that WILL change during play (as player levels up).

static Creature::Stats s_gingerbread [Creature::Count];
static Spell::Bitset s_gingerbread_spells [Creature::Count]; 
static std::unordered_set<NameHash> s_gingerbread_tags [Creature::Count];

void parse_spell_string (Spell::Bitset & spell_bitset, std::string const & spell_string);
void parse_tag_string (std::unordered_set<NameHash> & tag_set, std::string const & tag_string);

void mix_gingerbread (
	Creature::Type type, Creature::Identity identity, float difficulty, float probability,
	char const * short_name, char const * long_name,
	int codepoint, char const * colour, Gender gender,
	int magic_skill, int max_hp, std::string spell_string,
	char const * tag_string)
{
	s_gingerbread[type] = { identity, difficulty, probability,
		short_name, long_name, colour, codepoint, magic_skill, max_hp, gender };
	parse_spell_string(s_gingerbread_spells[type], spell_string);
	parse_tag_string(s_gingerbread_tags[type], tag_string);
}

void parse_spell_string (Spell::Bitset & spell_bitset, std::string const & spell_string)
{
	std::stringstream ss(spell_string);

	std::string token;
	while (ss >> token)
	{
		Spell::Index spell = Spell::get_index_by_abbrev(token);
		assert(spell != Spell::None);
		spell_bitset.set(spell, true);
	}
}

void parse_tag_string (std::unordered_set<NameHash> & tag_set, std::string const & tag_string)
{
	tag_set.clear();

	if (!tag_string.empty())
	{
		std::stringstream ss(tag_string);
		std::string token;

		while (ss >> token)
		{
			tag_set.insert(NameHash(token.c_str()));
		}
	}
}

void init ()
{
	init_gingerbread(); // Implemented in Gingerbread.cpp
}

Stats& edit_player_stats()
{
	return s_gingerbread[Creature::Player];
}

//-------------------------------------------------------------------------------------------------

// Individual creatures are stored in the s_creatures array.
// The parallel arrays (s_creature_status, s_derived_stats, etc.) hold further information.
// The arrays are hidden but can be accessed with the functions such as creature_type()
// Just as with the gingerbread array, the first entry is reserved for the player.
// This means the Creature::Player constant applies to *both* gingerbread *and* g_creatures.

int constexpr c_max_creatures = 200;
static Creature::Instance s_creatures [c_max_creatures];
static Grid<int> s_creature_status; // [creature][status]
static Creature::DerivedStats s_derived_stats [c_max_creatures];
static Spell::Bitset s_spells_known [c_max_creatures];
static int s_max_creature_index;

int constexpr c_rest_turns_per_hp = 5;

std::vector<Creature::Handle> s_visible_creatures;
std::unordered_map<int,int> s_fainting_creatures; // and instigator for each

static Creature::Instance const & read_creature_instance (Creature::Handle creature)
{
	assert(creature.valid());
	return s_creatures[creature];
}

static Creature::Instance & edit_creature_instance (Creature::Handle creature)
{
	assert(creature.valid());
	return s_creatures[creature];
}

static Creature::Stats const & read_creature_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_gingerbread[creature.type()];
}

static Creature::Stats & edit_creature_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_gingerbread[creature.type()];
}

static Creature::DerivedStats const & read_derived_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_derived_stats[creature];
}

static Creature::DerivedStats & edit_derived_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_derived_stats[creature];
}

void clear ()
{
	// empty creature arrays
	for (Creature::Instance & c : s_creatures)
	{
		c = Creature::Instance{};
	}

	for (int i = 0; i < (int)Creature::Identity::Count; ++i)
	{
		s_identity_metadata[i] = {};
	}

	s_creature_status = make_grid(c_max_creatures, Status::Count, 0);

	s_max_creature_index = 0;

	s_visible_creatures.clear();
	s_visible_creatures.reserve(c_max_creatures);
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Simple accessor functions

bool Handle::valid () const
{
	return index >= 0
		&& index < s_max_creature_index
		&& s_creatures[index].type > Creature::None
		&& s_creatures[index].type < Creature::Count;
}

Creature::Type Handle::type () const
{
	return read_creature_instance(index).type;
}

Creature::Identity Handle::identity () const
{
	return read_creature_stats(index).identity;
}

std::string Handle::short_name () const
{
	return read_creature_stats(index).short_name;
}

std::string Handle::long_name () const
{
	return read_creature_stats(index).long_name;
}

Gender Handle::gender () const
{
	return read_creature_stats(index).gender;
}

int Handle::skill_magic () const
{
	return read_creature_stats(index).skill_magic;
}

int Handle::max_hp () const
{
	return read_creature_stats(index).max_hp;
}

int Handle::hp () const
{
	return read_creature_instance(index).hp;
}

float Handle::hp_percent() const
{
	return (float)(hp()) / (float)(max_hp());
}

bool Handle::is_hurt() const
{
	return hp() < max_hp();
}

Vec3 Handle::pos () const
{
	return read_creature_instance(index).pos;
}

bool Handle::has_status (Status::Index status) const
{
	return s_creature_status[index][status] > 0;
}

int Handle::status_severity (Status::Index status) const
{
	return s_creature_status[index][status];
}

int Handle::distractedness () const
{
	return read_derived_stats(index).distractedness;
}

int Handle::miscastiness () const
{
	return read_derived_stats(index).miscastiness;
}

int Handle::evasion () const
{
	return read_derived_stats(index).evasion;
}

int Handle::accuracy () const
{
	return read_derived_stats(index).accuracy;
}

int Handle::walk_failure () const
{
	return std::min(90, read_derived_stats(index).walk_failure);
}

bool Handle::knows_spell (Spell::Index spell) const
{
	Spell::Bitset const & spell_bitset = s_spells_known[index];
	return spell_bitset.test(spell);
}

bool Handle::has_tag (NameHash tag) const
{
	assert(valid());
	return s_gingerbread_tags[type()].count(tag) > 0;
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Complex accessor functions

bool Handle::is_player () const
{
	if (Handle(index).type() == Creature::Player)
	{
		assert(index == Creature::Player);
		return true;
	}
	else
	{
		assert(index != Creature::Player);
		return false;
	}
}

bool Handle::visible () const
{
	if (!valid())
	{
		return false;
	}

	World const& world = World::read();
	return world.is_visible(pos());
}

float Handle::miscast_rate_for_spell (Spell::Index spell) const
{
	// Miscastiness effectively applies a penalty to your magic skill.
	int effective_skill_magic = skill_magic() - miscastiness();

	return Spell::get_miscast_rate(spell, effective_skill_magic);
}

std::string Handle::status_string () const
{
	std::stringstream outs;
	int num_out = 0;
	int i = 0;
	while (i < Status::Count && num_out < 6)
	{
		Status::Index si = static_cast<Status::Index>(i);
		if (has_status(si))
		{
			if (num_out < 5)
			{
				int severity = status_severity(si);
				outs << Status::abbrev(si);
				outs << "(" << severity << ")  ";
			}
			else
			{
				outs << "...";
			}
			num_out += 1;
		}
		++ i;
	}
	return outs.str();
}

std::vector<Spell::Index> Handle::spells_known () const
{
	Spell::Bitset const & spell_bitset = s_spells_known[index];
	std::vector<Spell::Index> spell_list;
	for (int i = 0; i < Spell::Index::Count; i++)
	{
		if (spell_bitset.test(i))
		{
			spell_list.push_back(static_cast<Spell::Index>(i));
		}
	}
	return spell_list;
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Mutator functions

void Handle::take_damage (int damage, Creature::Handle instigator)
{
	Creature::Instance & c = edit_creature_instance(index);
	c.hp -= damage;

	if (is_player())
	{
		Player::stop_automove();
	}

	if (c.hp <= 0)
	{
		c.hp = 0;
		s_fainting_creatures.try_emplace(*this, instigator.valid() ? instigator.type() : Type::None);
	}
}

void Handle::move (Vec3 const & new_pos)
{
	edit_creature_instance(index).pos = new_pos;
}

void Handle::inflict_status (Status::Index status, int severity)
{
	if (!has_status(status))
	{
		s_creature_status[index][status] = severity;
	}
	else
	{
		s_creature_status[index][status] += severity;
	}

	if (s_creature_status[index][status] > Status::max_severity(status))
	{
		s_creature_status[index][status] = Status::max_severity(status);
	}

	update_derived_stats();
}

void Handle::reduce_status (Status::Index status, int reduction)
{
	if (!has_status(status))
	{
		//cerr << "Can't reduce non-afflicted status " << (int)the_status << endl;
		return;
	}
	else
	{
		s_creature_status[index][status] -= reduction;
		if (s_creature_status[index][status] <= 0)
		{
			s_creature_status[index][status] = 0;
			if (visible())
			{
				Status::print_cure_message(*this, status);
			}
		}
	}

	update_derived_stats();
}

void Handle::cure_status (Status::Index status)
{
	reduce_status(status, Status::max_severity(status));
}

void Handle::cure_all ()
{
	// blank all statuses (with no message)
	s_creature_status[index] = std::vector<int>(Status::Count, 0);
	edit_creature_instance(index).hp = max_hp();
}

void Handle::rest_step ()
{
	Creature::Instance& inst = edit_creature_instance(index);

	if (is_hurt()
		&& read_derived_stats(index).distractedness == 0)
	{
		++ inst.rest_turns;
		if (inst.rest_turns >= c_rest_turns_per_hp)
		{
			++ inst.hp;
			inst.rest_turns = 0;
		}
	}
	else
	{
		clear_rest_steps();
	}
}

void Handle::clear_rest_steps ()
{
	edit_creature_instance(index).rest_turns = 0;
}

void Handle::invalidate ()
{
	if (identity() != Identity::Generic)
	{
		if (s_identity_metadata[(int)identity()].current_handle == *this)
		{
			s_identity_metadata[(int)identity()].current_handle = Creature::None;
		}
	}

	s_creatures[index].type = Creature::None;
}

void Handle::update_derived_stats ()
{
	Creature::DerivedStats & ds = edit_derived_stats(index);
	
	ds = DerivedStats{};

	for (int i = 0; i < Status::Count; i++)
	{
		Status::Index si = static_cast<Status::Index>(i);
		if (has_status(si))
		{
			Status::apply_to_derived_stats(si, *this, ds);
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Creature Handle Iterator

HandleItr::HandleItr(int start_at)
	: current(start_at)
{
	while (!current.valid()
		&& current < s_max_creature_index)
	{
		++current;
	}
}

Creature::Handle HandleItr::get () const
{
	return current;
}

void HandleItr::advance ()
{
	++ current;
	while (!current.valid()
		&& current < s_max_creature_index)
	{
		++ current;
	}
}

bool HandleItr::finished () const
{
	return current >= s_max_creature_index;
}

//-------------------------------------------------------------------------------------------------
// Global Creature interface

const char* short_name_from_type (Creature::Type type)
{
	if (type >= 0 && type <= Creature::Type::Count)
	{
		return s_gingerbread[type].short_name;
	}
	return "no one";
}

const char* long_name_from_type (Creature::Type type)
{
	if (type >= 0 && type <= Creature::Type::Count)
	{
		return s_gingerbread[type].long_name;
	}
	return "no one";
}

Creature::Handle creature_at_pos (Vec3 pos)
{
	for (Creature::HandleItr itr(0); itr; ++itr)
	{
		if (itr->pos() == pos)
		{
			return *itr;
		}
	}

	return Creature::None;
}

Creature::Handle spawn_creature (Creature::Type type, Vec3 const & pos)
{
	if (type <= Creature::None || type >= Creature::Count)
	{
		DebugBreak("Spawning invalid creature type");
		return Creature::None;
	}

	// find creature number
	int new_index = c_invalid;
	for (int i = 0; i < s_max_creature_index; i++)
	{
		if (!Handle(i).valid())
		{
			new_index = i;
			break;
		}
	}

	if (new_index == c_invalid && s_max_creature_index < c_max_creatures)
	{
		new_index = s_max_creature_index;
		++ s_max_creature_index;
	}

	assert(new_index != c_invalid); // if this fails, increase creature memory budget

	// allocate new creature on the arrays
	s_creatures[new_index] =
	{
		type,
		s_gingerbread[type].max_hp,
		pos
	};
	s_spells_known[new_index] = s_gingerbread_spells[type];
	Handle(new_index).cure_all();
	Handle(new_index).update_derived_stats();

	Bot::init_brain(new_index);

	// update identity metadata
	Identity const identity = s_gingerbread[type].identity;
	if (identity != Identity::Generic)
	{
		IdentityMetadata& metadata = s_identity_metadata[(int)identity];

		if (metadata.current_handle != Creature::None)
		{
			// Force unspawn the old instance, and spawn the new one.
			std::cout << "Unspawning old instance of " << s_gingerbread[type].short_name
				<< " to spawn new instance.\n";
			metadata.current_handle.invalidate();
		}

		if (s_gingerbread[type].difficulty <= metadata.spawned_difficulty)
		{
			std::cout << "Warning: Spawning " << s_gingerbread[type].short_name
				<< " at lower or equal difficulty than last time.\n"
				<< " - New=" << s_gingerbread[type].difficulty
				<< ", old=" << metadata.spawned_difficulty << "\n";
		}

		metadata.current_handle = new_index;
		metadata.spawned_difficulty = s_gingerbread[type].difficulty;
	}

	// return the index of the new creature
	return Handle(new_index);
}

Creature::Type find_type_to_spawn (float target_difficulty)
{
	std::vector<Type> options;
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
		Stats const& stats = s_gingerbread[type];
		Identity const identity = stats.identity;

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

//-------------------------------------------------------------------------------------------------
// Visible creatures operations.
// We maintain a collection of creatures visible to the player to avoid iterating over the entire
// creature array and checking their visibility.
// We skip over position 0 in the array, which should always contain the player.

void update_visible_creatures ()
{
	s_visible_creatures.clear();
	assert(Handle(0).is_player()); // we are skipping index 0 on this premise
	for (Creature::HandleItr itr(1); itr; ++itr)
	{
		if (itr->visible())
		{
			s_visible_creatures.push_back(*itr);
		}
	}
}

void draw_creature (Creature::Handle creature, Draw::View const & view)
{
	Vec3 pos = creature.pos();
	if (view.contains_global_pos(pos))
	{
		Creature::Type const type = creature.type();
		int const code = s_gingerbread[type].codepoint;
		char const * creature_colour = s_gingerbread[type].colour ?
			s_gingerbread[type].colour : "white";

		if (Target::is_target(creature))
		{
			Draw::draw_tile_bg(code, pos.xy(), view, creature_colour, c_target_colour);
		}
		else
		{
			Draw::draw_tile(code, pos.xy(), view, creature_colour);
		}
	}
}

void draw_visible_creatures (Draw::View const & view)
{
	for (int i : s_visible_creatures)
	{
		draw_creature(i, view);
	}
}

void remove_defeated_creatures ()
{
	int num_removed = 0;

	for (std::pair<int,int> const& pair : s_fainting_creatures)
	{
		Handle creature = pair.first;
		Creature::Type const instigator_type = (Creature::Type)pair.second;

		if (creature.visible() || instigator_type == Type::Player)
		{
			if (creature.has_tag("Faint.Disappear"))
			{
				Draw::add_message(creature.long_name() + " disappears.");
			}
			else
			{
				Draw::add_message(Grammar::You(creature) + " " + Grammar::verbs("faint", creature) + ".");
			}
		}

		if (creature.is_player())
		{
			Player::set_game_over(instigator_type);
		}
		else
		{
			++num_removed;
			creature.invalidate();
		}
	}
	s_fainting_creatures.clear();

	if (num_removed > 0)
	{
		update_visible_creatures();
	}
}

std::vector<Creature::Handle> const & get_visible_creatures ()
{
	return s_visible_creatures;
}

} // namespace Creature
