#include "Creature.h"

#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "Bot.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Grid.h"
#include "Gingerbread.h"
#include "Math.h"
#include "PerfTimer.h"
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

// Individual creatures are stored in the s_creatures array.
// The parallel arrays (s_creature_status, s_derived_stats, etc.) hold further information.
// The arrays are hidden but can be accessed with the functions such as creature_type()
// Just as with the gingerbread array, the first entry is reserved for the player.
// This means the Creature::Player constant applies to *both* gingerbread *and* g_creatures.

int constexpr c_max_creatures = 200;
static Creature::Instance s_creatures [c_max_creatures];
static Grid<int> s_creature_status; // (creature, status)
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

static Gingerbread::Stats const & read_creature_stats (Creature::Handle creature)
{
	assert(creature.valid());
	return Gingerbread::read(creature.type());
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

void init ()
{
}

void clear ()
{
	// empty creature arrays
	for (int i = 0; i < c_max_creatures; ++i)
	{
		s_creatures[i] = Instance{};
		s_derived_stats[i] = DerivedStats{};
		s_spells_known[i] = Spell::Bitset{};
	}

	s_creature_status = Grid(c_max_creatures, Status::Count, 0);

	s_max_creature_index = 0;

	s_visible_creatures.clear();
	s_visible_creatures.reserve(c_max_creatures);

	s_fainting_creatures.clear();
	s_fainting_creatures.reserve(10);
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

NameHash Handle::identity () const
{
	return read_creature_stats(index).identity;
}

bool Handle::is_generic () const
{
	return identity() == c_IdentityGeneric;
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
	return s_creature_status.read(index, status) > 0;
}

int Handle::status_severity (Status::Index status) const
{
	return s_creature_status.read(index, status);
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
	return Gingerbread::has_tag(type(), tag);
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
				outs << std::format("[color={}]{}({})[/color]",
					Status::colour(si), Status::abbrev(si), status_severity(si));
			}
			else
			{
				outs << "[color=lighter grey]...[/color]";
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
	Spell::bitset_to_list(spell_bitset, spell_list);
	return spell_list;
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Mutator functions

void Handle::take_damage (int damage, Creature::Handle instigator)
{
	Creature::Instance & c = edit_creature_instance(index);
	c.hp -= damage;

	clear_rest_steps();

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

void Handle::heal_hp (int healing)
{
	Creature::Instance & c = edit_creature_instance(index);
	c.hp = std::min(c.hp + healing, max_hp());
}

void Handle::move (Vec3 const & new_pos)
{
	edit_creature_instance(index).pos = new_pos;
}

void Handle::inflict_status (Status::Index status, int severity)
{
	if (!has_status(status))
	{
		s_creature_status.edit(index, status) = severity;
	}
	else
	{
		s_creature_status.edit(index, status) += severity;
	}

	if (s_creature_status.read(index, status) > Status::max_severity(status))
	{
		s_creature_status.edit(index, status) = Status::max_severity(status);
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
		s_creature_status.edit(index, status) -= reduction;
		if (s_creature_status.read(index, status) <= 0)
		{
			s_creature_status.edit(index, status) = 0;
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
	s_creature_status.fill(0);
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
	if (identity() != c_IdentityGeneric)
	{
		Gingerbread::release_identity(*this);
	}

	s_creatures[index].type = Creature::None;
}

void Handle::reset_spells ()
{
	s_spells_known[index] = Gingerbread::read_spells(type());
}

void Handle::learn_spell (Spell::Index spell)
{
	assert(Spell::is_valid_index(spell));
	s_spells_known[index].set((int)spell);
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

Creature::Handle creature_at_pos (Vec3 pos)
{
	PerfTimer perf("creature_at_pos");

	for (Creature::HandleItr itr(0); itr; ++itr)
	{
		if (itr->pos() == pos)
		{
			return *itr;
		}
	}

	return Creature::None;
}

bool is_anyone_at (Vec3 pos)
{
	return creature_at_pos(pos) != Creature::None;
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
		Gingerbread::read(type).max_hp,
		pos
	};

	Creature::Handle new_creature(new_index);

	new_creature.reset_spells();
	new_creature.cure_all();
	new_creature.update_derived_stats();

	Bot::init_brain(new_creature);
	Gingerbread::claim_identity(new_creature);

	assert(new_creature.valid());

	// return the index of the new creature
	return new_creature;
}

// Helper function
void creature_drop_item(Vec3 pos, Creature::Type type)
{
	Item::Type item_type = Gingerbread::random_item_drop(type);
	switch (item_type)
	{
		case Item::BBBean:
			Item::spawn_bbb(pos);
			break;
		case Item::Notes:
			Item::spawn_notes(pos, type);
			break;
		case Item::PotionItem:
			Item::spawn_potion_by_level(pos, Gingerbread::read(type).difficulty);
			break;
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
			Player::gain_xp_for(creature.type());

			creature_drop_item(creature.pos(), creature.type());
			if (creature.has_tag("Drop.Notes"))
			{
				Item::spawn_notes(creature.pos(), creature.type());
			}

			creature.invalidate();
		}
	}
	s_fainting_creatures.clear();

	if (num_removed > 0)
	{
		update_visible_creatures();
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
		int const code = Gingerbread::read(type).codepoint;
		char const * creature_colour = Gingerbread::read(type).colour;

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

std::vector<Creature::Handle> const & get_visible_creatures ()
{
	return s_visible_creatures;
}

} // namespace Creature
