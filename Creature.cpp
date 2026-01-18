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

namespace Creature
{

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

void init ()
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

std::vector<Creature::Handle> s_visible_creatures;

static Creature::Stats & get_creature_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_creatures[creature];
}

static Creature::DerivedStats & get_derived_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return s_derived_stats[creature];
}

void clear ()
{
	// empty creature arrays
	for (Creature::Stats & c : s_creatures)
	{
		c.type = Creature::None;
	}

	s_creature_status = make_grid(MAX_CREATURES, Status::Count, 0);

	s_max_creature_index = 0;

	s_visible_creatures.clear();
	s_visible_creatures.reserve(MAX_CREATURES);
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Simple accessor functions

bool Handle::valid () const
{
	return index >= 0
		&& index < s_max_creature_index
		&& s_creatures[index].type != Creature::None;
}

Creature::Type Handle::type () const
{
	return get_creature_stats(index).type;
}

std::string Handle::name () const
{
	return get_creature_stats(index).name;
}

Gender Handle::gender () const
{
	return get_creature_stats(index).gender;
}

int Handle::skill_magic () const
{
	return get_creature_stats(index).skill_magic;
}

int Handle::max_hp () const
{
	return get_creature_stats(index).max_hp;
}

int Handle::hp () const
{
	return get_creature_stats(index).hp;
}

float Handle::hp_percent() const
{
	return (float)(hp()) / (float)(max_hp());
}

Vec2 const & Handle::pos () const
{
	return get_creature_stats(index).pos;
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
	return get_derived_stats(index).distractedness;
}

int Handle::miscastiness () const
{
	return get_derived_stats(index).miscastiness;
}

int Handle::evasion () const
{
	return get_derived_stats(index).evasion;
}

int Handle::accuracy () const
{
	return get_derived_stats(index).accuracy;
}

bool Handle::knows_spell (Spell::Index spell) const
{
	Spell::Bitset const & spell_bitset = s_spells_known[index];
	return spell_bitset.test(spell);
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

	Map const & map = g_map();
	if (!map.contains(pos()))
	{
		return false; // it's off the map
	}

	Visibility v = g_map().get_visibility(pos());
	if (v == Visibility::Visible)
	{
		return true;
	}
	else
	{
		return false;
	}
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

void Handle::take_damage (int damage)
{
	Creature::Stats & c = get_creature_stats(index);
	c.hp -= damage;

	// todo: die, I guess
	// or we could come back at the end of the round to "collect" dead creatures.
}

void Handle::move (Vec2 const & new_pos)
{
	get_creature_stats(index).pos = new_pos;
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
			// Status::print_cure_message(creature, status); // todo maybe
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
	get_creature_stats(index).hp = max_hp();
}

void Handle::update_derived_stats ()
{
	Creature::DerivedStats & ds = get_derived_stats(index);
	
	ds.distractedness = 0;
	ds.miscastiness = 0;
	ds.evasion = 0;
	ds.accuracy = 0;
	ds.shield_strength = 0;

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

HandleItr::HandleItr ()
	: current(0)
{ }

Creature::Handle HandleItr::get () const
{
	return current;
}

void HandleItr::advance()
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

Creature::Handle creature_at_pos (Vec2 pos)
{
	// search creature array
	for (Creature::Handle i = 0; i < s_max_creature_index; ++ i)
	{
		if (i.pos() == pos)
		{
			return i;
		}
	}

	return Creature::None;
}

Creature::Handle spawn_creature (Creature::Type type, Vec2 const & pos)
{
	// find creature number
	int new_index = -1;
	for (int i = 0; i < s_max_creature_index; i++)
	{
		if (!Handle(i).valid())
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
	Handle(new_index).cure_all();
	Handle(new_index).update_derived_stats();

	// return the index of the new creature
	return Handle(new_index);
}

//-------------------------------------------------------------------------------------------------
// We maintain a collection of creatures visible to the player to avoid iterating over the entire
// creature array and checking their visibility.
// We skip over position 0 in the array, which should always contain the player.

void update_visible_creatures ()
{
	s_visible_creatures.clear();
	assert(Handle(0).is_player()); // we are skipping index 0 on this premise
	for (Creature::Handle creature = 1; creature < s_max_creature_index; ++ creature)
	{
		if (creature.visible())
		{
			s_visible_creatures.push_back(creature);
		}
	}
}

void draw_creature (Creature::Handle creature, DrawView const & view)
{
	Vec2 const & pos = creature.pos();
	if (view.contains_global_pos(pos))
	{
		int code = s_creatures[creature].codepoint;
		if (creature_is_targeted(creature))
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
	for (int i : s_visible_creatures)
	{
		draw_creature(i, view);
	}
}

std::vector<Creature::Handle> const & get_visible_creatures ()
{
	return s_visible_creatures;
}

} // namespace Creature
