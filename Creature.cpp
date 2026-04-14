#include "Creature.h"

#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "Ability.h"
#include "Bot.h"
#include "Cloud.h"
#include "Colour.h"
#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Game.h"
#include "Grammar.h"
#include "Grid.h"
#include "Gingerbread.h"
#include "Inventory.h"
#include "Math.h"
#include "PerfTimer.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Spell.h"
#include "Squad.h"
#include "Status.h"
#include "Target.h"
#include "VectorUtil.h"
#include "World.h"

namespace Creature
{

//-------------------------------------------------------------------------------------------------

constexpr int c_VisionNormal = 8;
constexpr int c_VisionShort = 3;

// Individual creatures are stored in the s_creatures array.
// The parallel arrays (s_creature_status, s_derived_stats, etc.) hold further information.
// The arrays are hidden but can be accessed with the functions such as creature_type()
// Just as with the gingerbread array, the first entry is reserved for the player.
// This means the Creature::Player constant applies to *both* gingerbread *and* g_creatures.

Creature::Instance s_creatures [c_MaxCreatures];
Grid<int> s_creature_status; // (creature, status)
Creature::DerivedStats s_derived_stats [c_MaxCreatures];
int s_max_creature_index;

Creature::HandleList s_visible_creatures;
std::unordered_map<int,Damage::Cause> s_fainting_creatures; // and how they fainted

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
	for (int i = 0; i < c_MaxCreatures; ++i)
	{
		s_creatures[i] = Instance{};
		s_derived_stats[i] = DerivedStats{};
	}

	s_creature_status = Grid(c_MaxCreatures, Status::Count, 0);

	s_max_creature_index = 0;

	s_visible_creatures.clear();
	s_visible_creatures.reserve(c_MaxCreatures);

	s_fainting_creatures.clear();
	s_fainting_creatures.reserve(c_MaxCreatures);
}

void serialize (ISerializer& s)
{
	s.srz_grid(s_creature_status, "s_creature_status");

	s.srz_int(s_max_creature_index);
	for (int i = 0; i < s_max_creature_index; ++i)
	{
		s.srz_value(s_creatures[i]);

		// Don't save/load derived stats; just regenerate them.
		if (s.is_load())
		{
			Creature::Handle handle(i);
			if (handle.valid())
			{
				Creature::Handle(i).update_derived_stats();
			}
		}
	}

	s.srz_vector(s_visible_creatures, "s_visible_creatures");

	// Shouldn't need to serialize because it will be empty by end of turn:
	assert(s_fainting_creatures.empty());
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

char const* Handle::short_name () const
{
	return read_creature_stats(index).short_name;
}

char const* Handle::long_name () const
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

int Handle::num_spells () const
{
	assert(valid());
	Spell::Bitset const & spell_bitset = s_creatures[index].spells;
	return (int)spell_bitset.count();
}

bool Handle::knows_spell (Spell::Index spell) const
{
	assert(valid());
	Spell::Bitset const & spell_bitset = s_creatures[index].spells;
	return spell_bitset.test(spell);
}

int Handle::num_abilities () const
{
	assert(valid());
	return Util::Size(Gingerbread::read_abilities(type()));
}

bool Handle::has_ability (Ability::Index ability) const
{
	assert(valid());
	return Util::Contains(Gingerbread::read_abilities(type()), ability);
}

std::vector<Ability::Index> const& Handle::ability_list () const
{
	assert(valid());
	return Gingerbread::read_abilities(type());
}

bool Handle::has_tag (Tag tag) const
{
	assert(valid());
	return Gingerbread::has_tag(type(), tag);
}

bool Handle::has_flag (Flag flag) const
{
	assert(valid());
	return read_creature_instance(index).flags.test((size_t)flag);
}

bool Handle::ready_to_move () const
{
	return !has_flag(Flag::MoveDelay);
}

bool Handle::has_squad () const
{
	return read_creature_instance(index).squad_id != c_Invalid;
}

Creature::HandleList& Handle::squad_members () const
{
	assert(has_squad());
	int const squad_id = read_creature_instance(index).squad_id;
	return Squad::get_squad(squad_id);
}

Creature::Handle Handle::squad_leader () const
{
	int const squad_id = read_creature_instance(index).squad_id;
	if (squad_id == c_Invalid)
	{
		return c_Invalid;
	}

	return Squad::get_squad(squad_id).at(0);
}

bool Handle::has_squad_leader () const
{
	return has_squad() && squad_leader() != *this;
}

bool Handle::is_squad_leader () const
{
	return has_squad() && squad_leader() == *this;
}

bool Handle::has_item () const
{
	assert(valid());

	if (is_player())
	{
		return Inventory::read().has_item();
	}

	return read_creature_instance(index).carried_item != c_Invalid;
}

Item::Handle Handle::peek_item () const
{
	assert(valid());

	if (is_player())
	{
		return has_item() ?
			Inventory::read().peek_item(0) :
			Item::Handle(c_Invalid);
	}

	return read_creature_instance(index).carried_item;
}

bool Handle::is_immune (Damage::Type damage_type) const
{
	return Gingerbread::read_resistance(type(), damage_type) == 0.0f;
}

bool Handle::resists (Damage::Type damage_type) const
{
	return Gingerbread::read_resistance(type(), damage_type) < 1.0f;
}

bool Handle::is_friend (Creature::Handle other_creature) const
{
	// For now, this is pretty simple:
	return is_player() == other_creature.is_player();
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Complex accessor functions

char const* Handle::colour () const
{
	assert(valid());
	if (has_tag(Tag::Colour_Rainbow))
	{
		int const x = (pos().x / 3) +
			(pos().y / 7) + 
			(Game::get_turn_number() / 12) +
			(index * 4);
		return Colour::rainbow(x);
	}
	else
	{
		return Gingerbread::read(type()).colour;
	}
}

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

bool Handle::finds_pos_hazardous (Vec3 pos) const
{
	Cloud::Type cloud = World::read().get_cloud(pos);
	if (cloud != Cloud::None)
	{
		if (Cloud::hazardous_for(cloud, *this))
		{
			return true;
		}
	}

	return false;
}

int Handle::vision () const
{
	if (has_tag(Tag::Vision_Short))
	{
		return c_VisionShort;
	}
	else if (is_player())
	{
		return Player::c_VisionRadius;
	}
	else
	{
		return c_VisionNormal;
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
		if (has_status(si)) // && !Status::hidden(si)
		{
			if (num_out < 5)
			{
				if (Status::show_number(si))
				{
					outs << std::format("[color={}]{}({})[/color] ",
						Status::colour(si), Status::abbrev(si), status_severity(si));
				}
				else
				{
					outs << std::format("[color={}]{}[/color] ",
						Status::colour(si), Status::abbrev(si));
				}
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

Spell::TempList Handle::spells_known () const
{
	Spell::Bitset const & spell_bitset = s_creatures[index].spells;
	return Spell::bitset_to_temp_list(spell_bitset);
}

//-------------------------------------------------------------------------------------------------
// Creature Handle - Mutator functions

void Handle::take_damage (Damage::Packet const& dmg)
{
	Creature::Instance & c = edit_creature_instance(index);

	float const factor = Gingerbread::read_resistance(type(), dmg.type);
	int new_damage = Math::RoundToInt((float)dmg.amount * factor);
	
	// Minimum 1 damage if not fully resisted.
	if (factor > 0.0f && dmg.amount > 0)
	{
		new_damage = std::max(new_damage, 1);
	}

	if (new_damage < dmg.amount)
	{
		if (new_damage > 0)
		{
			Draw::creature_message(*this, std::format("{} {}.",
				Grammar::You(*this), Grammar::verbs("resist",*this)));
		}
		else
		{
			Draw::creature_message(*this, std::format("{} unharmed.",
				Grammar::You_are(*this)));
		}
	}
	else if (new_damage > dmg.amount)
	{
		Draw::creature_message(*this, std::format("{} strongly affected!",
			Grammar::You_are(*this)));
	}

	if (new_damage > 0)
	{
		c.hp -= new_damage;

		clear_rest_steps();

		if (c.hp <= 0)
		{
			c.hp = 0;
			s_fainting_creatures.try_emplace(index, dmg.cause);
		}
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
	for (int i = 0; i < Status::Count; ++i)
	{
		s_creature_status.edit(index, i) = 0;
	}
	update_derived_stats();

	edit_creature_instance(index).hp = max_hp();
}

void Handle::endround ()
{
	if (has_tag(Tag::Trail_Slime))
	{
		World::edit().try_add_cloud(pos(), Cloud::Slime, Random::in_range(21,24));
	}

	Status::do_endround(*this);
}

void Handle::rest_step ()
{
	Creature::Instance& inst = edit_creature_instance(index);

	clear_flag(Flag::MoveDelay);

	if (is_hurt()
		&& read_derived_stats(index).distractedness == 0)
	{
		++ inst.rest_turns;
		if (inst.rest_turns >= c_RestTurnsPerHp)
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

void Handle::destroy ()
{
	if (identity() != c_IdentityGeneric)
	{
		Gingerbread::release_identity(*this);
	}

	remove_from_squad();
	Ability::clear_cooldowns(*this);

	s_creatures[index].type = Creature::None;
}

void Handle::reset_spells ()
{
	s_creatures[index].spells = Gingerbread::read_spells(type());

	if (has_tag(Tag::Spells_Random))
	{
		learn_random_spells();
	}

	Bot::notify_attacks_changed(*this);
}

void Handle::learn_spell (Spell::Index spell)
{
	assert(Spell::is_valid_index(spell));
	s_creatures[index].spells.set((int)spell);
	Bot::notify_attacks_changed(*this);
}

void Handle::learn_random_spells()
{
	Spell::TempList options;
	Spell::TempList damaging_options;
	options.reserve(Spell::Count);
	damaging_options.reserve(Spell::Count / 2);

	int const skill = skill_magic();
	int const min = skill - 50;
	int const max = skill + 25;

	bool has_damaging = false;
	for (int i = 0; i < Spell::Count; ++i)
	{
		Spell::Index const spell = (Spell::Index)i;
		int const difficulty = Spell::get_difficulty(spell);
		if ((difficulty >= min || spell == Spell::Stupefy) &&
			difficulty <= max &&
			spell != Spell::Megabolt)
		{
			if (!knows_spell(spell))
			{
				options.push_back(spell);
				if (Spell::is_damaging(spell))
				{
					damaging_options.push_back(spell);
				}
			}
			else if (Spell::is_damaging(spell))
			{
				has_damaging = true;
			}
		}
	}

	// If we don't have a damaging spell, add at least one.
	if (!has_damaging)
	{
		if (!damaging_options.empty())
		{
			Spell::Index const spell = Random::from_vector(damaging_options);
			s_creatures[index].spells.set((int)spell);
			Util::RemoveSwapFirstMatchingItem(options, spell);
		}
		else
		{
			s_creatures[index].spells.set((int)Spell::Vermillious);
		}
	}

	// Now learn a couple random spells.
	int const num_to_learn = Random::in_range(2,4) - num_spells();
	for (int i = 0;
		i < num_to_learn && !options.empty();
		++i)
	{
		int const r = Random::index(options);
		Spell::Index const spell = options[r];
		s_creatures[index].spells.set((int)spell);
		Util::RemoveSwap(options, r);
	}
}

void Handle::set_flag (Flag flag)
{
	assert(valid());
	s_creatures[index].flags.set((size_t)flag, true);
}

void Handle::clear_flag (Flag flag)
{
	assert(valid());
	s_creatures[index].flags.set((size_t)flag, false);
}

void Handle::set_squad (int new_squad_id)
{
	assert(valid());
	int& squad_id = s_creatures[index].squad_id;

	if (squad_id == new_squad_id)
	{
		return;
	}

	if (squad_id != c_Invalid)
	{
		Squad::remove_creature(squad_id, *this);
	}

	squad_id = new_squad_id;
	Squad::add_creature(new_squad_id, *this);
}

void Handle::remove_from_squad ()
{
	assert(valid());
	int& squad_id = s_creatures[index].squad_id;

	if (squad_id != c_Invalid)
	{
		Squad::remove_creature(squad_id, *this);
	}
	squad_id = c_Invalid;
}

void Handle::push_item (Item::Handle item)
{
	assert(valid());

	// Item must not already be in a different stack.
	assert(item.next_in_stack() == c_Invalid);

	item.stack_onto(read_creature_instance(index).carried_item);
	edit_creature_instance(index).carried_item = item;
}

Item::Handle Handle::pop_item ()
{
	if (!has_item())
	{
		return Item::Handle(c_Invalid);
	}
	else
	{
		if (is_player())
		{
			int const slot = Inventory::read().random_slot();
			return Inventory::edit().pop_item(slot);
		}

		return Item::unstack(edit_creature_instance(index).carried_item);
	}
}

void Handle::drop_all_items ()
{
	// They'll end up stacked the other way, but that's ok
	Item::Handle item = pop_item();
	while (item != c_Invalid)
	{
		World::edit().add_item(pos(), item);
		item = pop_item();
	}
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

	if (has_tag(Creature::Tag::Evade_High))
	{
		ds.evasion += 25;
	}
	else if (has_tag(Creature::Tag::Evade_Medium))
	{
		ds.evasion += 12;
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

bool is_valid_type (Creature::Type type)
{
	return type > Creature::None && type < Creature::Count;
}

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
	return creature_at_pos(pos).valid();
}

Creature::Handle spawn_creature (Creature::Type type, Vec3 const & pos)
{
	if (type <= Creature::None || type >= Creature::Count)
	{
		DebugBreak("Spawning invalid creature type");
		return Creature::None;
	}

	// find creature number
	int new_index = c_Invalid;
	for (int i = 0; i < s_max_creature_index; i++)
	{
		if (!Handle(i).valid())
		{
			new_index = i;
			break;
		}
	}

	if (new_index == c_Invalid && s_max_creature_index < c_MaxCreatures)
	{
		new_index = s_max_creature_index;
		++ s_max_creature_index;
	}

	assert(new_index != c_Invalid); // if this fails, increase creature memory budget

	// allocate new creature on the arrays
	s_creatures[new_index] =
	{
		.pos = pos,
		.type = type,
		.hp = Gingerbread::read(type).max_hp,
	};

	Creature::Handle new_creature(new_index);

	new_creature.reset_spells();
	new_creature.cure_all();
	new_creature.update_derived_stats();

	Bot::reset_brain(new_creature);
	Gingerbread::claim_identity(new_creature);

	Item::Handle item = Gingerbread::make_item_for_creature(type);
	if (item.valid())
	{
		new_creature.push_item(item);
	}

	assert(new_creature.valid());

	// return the index of the new creature
	return new_creature;
}

void remove_defeated_creatures ()
{
	int num_removed = 0;

	for (std::pair<int,Damage::Cause> const& pair : s_fainting_creatures)
	{
		Handle creature = pair.first;
		Damage::Cause const& cause = pair.second;

		Creature::Type const instigator_type = (cause.type == Damage::Cause::Creature) ?
			(Creature::Type)cause.index : Creature::None;

		if (creature.visible() || instigator_type == Type::Player)
		{
			if (creature.has_tag(Tag::Faint_Disappear))
			{
				Draw::add_message(std::format("{} disappears.", creature.long_name()));
			}
			else
			{
				Draw::add_message(Grammar::You(creature) + " " + Grammar::verbs("faint", creature) + ".");
			}
		}

		if (creature.is_player())
		{
			Player::set_defeated(cause);
		}
		else
		{
			if (creature.type() == Creature::MarySue &&
				!Player::is_game_over())
			{
				Player::set_won();
			}

			++num_removed;
			Player::gain_xp_for(creature.type());

			creature.drop_all_items();

			creature.destroy();
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
		char const * creature_colour = creature.colour();

		if (Crosshair::is_target(creature))
		{
			Draw::draw_tile_bg(code, pos.xy(), view, creature_colour, Crosshair::colour());
		}
		else
		{
			Draw::draw_tile(code, pos.xy(), view, creature_colour);
		}
	}
}

void draw_visible_creatures (Draw::View const & view)
{
	// Cheat mode: draw ALL the creatures.
	if (Draw::los_cheat_enabled())
	{
		for (Creature::HandleItr itr(1); itr; ++itr)
		{
			draw_creature(*itr, view);
		}
		return;
	}

	for (int i : s_visible_creatures)
	{
		draw_creature(i, view);
	}
}

Creature::HandleList const & get_visible_creatures ()
{
	return s_visible_creatures;
}

bool has_visible_enemy ()
{
	return !s_visible_creatures.empty();
}

} // namespace Creature
