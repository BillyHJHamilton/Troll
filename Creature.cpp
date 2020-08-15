#include "BearLibTerminal.h"

#include "Creature.h"
#include "Draw.h"
#include "Global.h"
#include "Map.h"
#include "Spell.h"
#include "Status.h"
#include "Target.h"

#include <cassert>
#include <sstream>

//-------------------------------------------------------------------------------------------------

// The gingerbread array stores prototype objects which can be copied to spawn creatures
// of different kinds.  (It is called gingerbread because they are like cookie cutters.)
// Note that the first entry in the array is reserved for the player.

static Creature::Stats s_gingerbread [Creature::Count];
static Spell::Bitset s_gingerbread_spells [Creature::Count]; 

void parse_spell_string (Spell::Bitset & spell_bitset, std::string const & spell_string);

void mix_gingerbread (Creature::Type type, char const * name, int codepoint, Gender gender, int magic, int hp, std::string spell_string)
{
	s_gingerbread[type] = {type, name, codepoint, gender, magic, hp, hp, {0,0}};
	parse_spell_string(s_gingerbread_spells[type], spell_string);
}

void init_creatures ()
{
	//																			  mag  hp  spells
	mix_gingerbread(Creature::Player,		 "Player",       '@', Gender::Female, 10,  10, "VM FP TA");
	mix_gingerbread(Creature::Neville_0,	 "Neville",		 'N', Gender::Male,	  0,   10, "VM FP");
	mix_gingerbread(Creature::ColinCreevy_0, "Colin Creevy", 'C', Gender::Male,   10,  10, "VM TA");
}

void parse_spell_string (Spell::Bitset & spell_bitset, std::string const & spell_string)
{
	int i = 0;

	std::stringstream ss(spell_string);

	std::string token;
	while (ss >> token)
	{
		Spell::Index spell = Spell::get_index_by_abbrev(token);
		assert(spell != Spell::None);
		spell_bitset.set(spell, true);
	}
}

//-------------------------------------------------------------------------------------------------

// Individual creatures are stored in the s_creatures array.
// The parallel arrays (s_creature_status, s_derived_stats, etc.) hold further information.
// The arrays are hidden but can be accessed with the functions such as creature_type()
// Just as with the gingerbread array, the first entry is reserved for the player.
// This means the Creature::Player constant applies to *both* gingerbread *and* g_creatures.

int constexpr MAX_CREATURES = 200;
static Creature::Stats s_creatures [MAX_CREATURES];
static Grid<int> s_creature_status; // [creature][status]
static Creature::DerivedStats s_derived_stats [MAX_CREATURES];
static Spell::Bitset s_spells_known [MAX_CREATURES];
static int s_max_creature_index;

static Creature::Stats & get_creature (int index)
{
	assert(creature_valid(index));
	return s_creatures[index];
}

static Creature::DerivedStats & get_derived_stats (int index)
{
	assert(creature_valid(index));
	return s_derived_stats[index];
}

void clear_creatures ()
{
	// empty creature arrays
	for (Creature::Stats & c : s_creatures)
	{
		c.type = Creature::None;
	}

	s_creature_status = make_grid(MAX_CREATURES, Status::Count, 0);

	s_max_creature_index = 0;

	g_visible_creatures.clear();
	g_visible_creatures.reserve(MAX_CREATURES);
}

//-------------------------------------------------------------------------------------------------
// Iterator over valid creature indices

Creature::IndexItr::IndexItr ()
	: current(0)
{
}

int Creature::IndexItr::get () const
{
	return current;
}

void Creature::IndexItr::advance ()
{
	++ current;
	while (!creature_valid(current) && current < s_max_creature_index)
	{
		++ current;
	}
}

bool Creature::IndexItr::finished () const
{
	return current >= s_max_creature_index;
}

//-------------------------------------------------------------------------------------------------
// Simple accessor functions

bool creature_valid(int creature_index)
{
	return creature_index >= 0
		&& creature_index < s_max_creature_index
		&& s_creatures[creature_index].type != Creature::None;
}

Creature::Type creature_type (int creature_index)
{
	return get_creature(creature_index).type;
}

std::string creature_name (int creature_index)
{
	return get_creature(creature_index).name;
}

Gender creature_gender (int creature_index)
{
	return get_creature(creature_index).gender;
}

int creature_skill_magic (int creature_index)
{
	return get_creature(creature_index).skill_magic;
}

int creature_max_hp(int creature_index)
{
	return get_creature(creature_index).max_hp;
}

int creature_hp(int creature_index)
{
	return get_creature(creature_index).hp;
}

Vec2 const & creature_pos (int creature_index)
{
	return get_creature(creature_index).pos;
}

bool creature_has_status (int creature_index, Status::Index status)
{
	return s_creature_status[creature_index][status] > 0;
}

int creature_status_severity (int creature_index, Status::Index status)
{
	return s_creature_status[creature_index][status];
}

int creature_distractedness (int creature_index)
{
	return get_derived_stats(creature_index).distractedness;
}

int creature_miscastiness (int creature_index)
{
	return get_derived_stats(creature_index).miscastiness;
}

int creature_evasion (int creature_index)
{
	return get_derived_stats(creature_index).evasion;
}

int creature_accuracy (int creature_index)
{
	return get_derived_stats(creature_index).accuracy;
}

bool creature_knows_spell (int creature_index, Spell::Index spell)
{
	Spell::Bitset const & spell_bitset = s_spells_known[creature_index];
	return spell_bitset.test(spell);
}

//-------------------------------------------------------------------------------------------------
// More complex accessor functions

bool creature_visible(int creature_index)
{
	if (!creature_valid(creature_index))
	{
		return false;
	}

	Map const & map = g_map();
	Vec2 pos = creature_pos(creature_index);

	if (!map.contains(pos))
	{
		return false; // it's off the map
	}

	Visibility v = g_map().get_visibility(pos);
	if (v == Visibility::Visible)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int creature_at_pos (Vec2 pos)
{
	// search creature array
	for (int i = 0; i < s_max_creature_index; i++)
	{
		if (creature_pos(i) == pos)
		{
			return i;
		}
	}

	return Creature::None;
}

bool creature_is_player (int creature_index)
{
	if (creature_type(creature_index) == Creature::Player)
	{
		assert(creature_index == Creature::Player);
		return true;
	}
	else
	{
		assert(creature_index != Creature::Player);
		return false;
	}
}

float creature_miscast_rate_for_spell (int creature_index, Spell::Index spell)
{
	// Miscastiness effectively applies a penalty to your magic skill.
	int miscastiness = creature_miscastiness(creature_index);
	int skill_magic = creature_skill_magic(creature_index);
	int effective_skill_magic = skill_magic - miscastiness;

	return Spell::get_miscast_rate(spell, effective_skill_magic);
}

std::string creature_status_string (int creature_index)
{
	std::stringstream outs;
	int num_out = 0;
	int i = 0;
	while (i < Status::Count && num_out < 6)
	{
		Status::Index si = static_cast<Status::Index>(i);
		if (creature_has_status(creature_index, si))
		{
			if (num_out < 5)
			{
				int severity = creature_status_severity(creature_index, si);
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

std::vector<Spell::Index> creature_spells_known (int creature_index)
{
	Spell::Bitset const & spell_bitset = s_spells_known[creature_index];
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
// Mutator functions

int spawn_creature (Creature::Type type, Vec2 const & pos)
{
	// find creature number
	int new_index = -1;
	for (int i = 0; i < s_max_creature_index; i++)
	{
		if (!creature_valid(i))
		{
			new_index = i;
			break;
		}
	}

	if (new_index == -1 && s_max_creature_index < MAX_CREATURES)
	{
		new_index = s_max_creature_index;
		++ s_max_creature_index;
	}

	assert(new_index != -1); // if this fails, increase creature memory budget

	// allocate new creature on the arrays
	s_creatures[new_index] = s_gingerbread[type];
	s_creatures[new_index].pos = pos;
	s_spells_known[new_index] = s_gingerbread_spells[type];
	creature_cure_all(new_index);
	update_derived_stats(new_index);

	// return the index of the new creature
	return new_index;
}

void damage_creature (int creature_index, int damage)
{
	Creature::Stats & c = get_creature(creature_index);
	c.hp -= damage;

	// todo: die, I guess
	// or we could come back at the end of the round to "collect" dead creatures.
}

void move_creature (int creature_index, Vec2 const & new_pos)
{
	get_creature(creature_index).pos = new_pos;
}

void inflict_status (int creature_index, Status::Index status, int severity)
{
	if (!creature_has_status(creature_index, status))
	{
		s_creature_status[creature_index][status] = severity;
	}
	else
	{
		s_creature_status[creature_index][status] += severity;
	}

	if (s_creature_status[creature_index][status] > Status::max_severity(status))
	{
		s_creature_status[creature_index][status] = Status::max_severity(status);
	}

	update_derived_stats(creature_index);
}

void reduce_status (int creature_index, Status::Index status, int reduction)
{
	if (!creature_has_status(creature_index, status))
	{
		//cerr << "Can't reduce non-afflicted status " << (int)the_status << endl;
		return;
	}
	else
	{
		s_creature_status[creature_index][status] -= reduction;
		if (s_creature_status[creature_index][status] <= 0)
		{
			s_creature_status[creature_index][status] = 0;
			// Status::print_cure_message(creature_index, status); // todo maybe
		}
	}

	update_derived_stats(creature_index);
}

void cure_status (int creature_index, Status::Index status)
{
	reduce_status(creature_index, status, Status::max_severity(status));
}

void creature_cure_all (int creature_index)
{
	// blank all statuses (with no message)
	s_creature_status[creature_index] = std::vector<int>(Status::Count, 0);
	get_creature(creature_index).hp = get_creature(creature_index).max_hp;
}

void update_derived_stats (int creature_index)
{
	Creature::DerivedStats & ds = get_derived_stats(creature_index);
	
	ds.distractedness = 0;
	ds.miscastiness = 0;
	ds.evasion = 0;
	ds.accuracy = 0;
	ds.shield_strength = 0;

	for (int i = 0; i < Status::Count; i++)
	{
		Status::Index si = static_cast<Status::Index>(i);
		if (creature_has_status(creature_index, si))
		{
			Status::apply_to_derived_stats(si, creature_index, ds);
		}
	}
}

//-------------------------------------------------------------------------------------------------
// We maintain a collection of creatures visible to the player to avoid iterating over the entire
// creature array and checking their visibility.
// We skip over position 0 in the array, which should always contain the player.

std::vector<int> g_visible_creatures; // by index

void update_visible_creatures ()
{
	g_visible_creatures.clear();
	assert(creature_is_player(0)); // we are skipping index 0 on this premise
	for (int i = 1; i < s_max_creature_index; i++)
	{
		if (creature_visible(i))
		{
			g_visible_creatures.push_back(i);
		}
	}
}

void draw_creature (int creature_index, DrawView const & view)
{
	Vec2 const & pos = creature_pos(creature_index);
	if (view.contains_global_pos(pos))
	{
		int code = s_creatures[creature_index].codepoint;
		if (creature_is_targeted(creature_index))
		{
			draw_tile_bg(code, pos, view, "white", TARGET_COLOUR);
		}
		else
		{
			draw_tile(code, pos, view, "white");
		}
	}
}

void draw_visible_creatures (DrawView const & view)
{
	for (int i : g_visible_creatures)
	{
		draw_creature(i, view);
	}
}

