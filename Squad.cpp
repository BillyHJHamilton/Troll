#include "Squad.h"

#include "Creature.h"
#include "Gingerbread.h"
#include "Math.h"
#include "Serialize.h"
#include "Spawn.h"
#include "VectorUtil.h"

#include <vector>

namespace Squad
{

//-------------------------------------------------------------------------------------------------
// Data

static std::vector<Squad::Definition> const s_squads =
{
	{ .debug_name="Gnome Squad", .difficulty=1.0f, .probability=0.4f,
	  .habitats=0x1,  // set as bits, TODO: How should this be done?
	  .flags=f_Repeat, .members={
		{Creature::Gnome, 2,5},
	}},

	{ .debug_name="Streeler Squad", .difficulty=1.5f, .probability=90.3f,
	  .habitats=0x2,  // set as bits, not in Hogwarts
	  .flags=f_Repeat, .members={
		{Creature::Streeler, 2,3},  // if 3 streelers aren't a problem, 10 wouldn't be
	}},

	{ .debug_name="Puff Posse", .difficulty=1.5f, .probability=0.1f,
	  .habitats=0x1,  // set as bits
	  .flags=f_Repeat, .members={
		{Creature::Hufflepuff_1, 3,4}
	}},

	{ .debug_name="Crabbe and Goyle", .difficulty=3.0f, .probability=1.0f,
	  .habitats=0x1,  // set as bits
	  .flags=f_None, .members={
		{Creature::Crabbe_3},
		{Creature::Goyle_3},
	}},

	{ .debug_name="Crab Squad", .difficulty=3.0f, .probability=0.3f,
	  .habitats=0x3,  // set as bits, Hogwarts and trap
	  .flags=f_Repeat, .members={
		{Creature::BigFireCrab},
		{Creature::FireCrab, 2,3},
	}},

	{ .debug_name="Imp Nest", .difficulty=3.0f, .probability=0.3f,
	  .habitats=0x1,  // set as bits
	  .flags=f_Repeat, .members={
		{Creature::Imp, 2,3},
	}},

	{ .debug_name="Doxy Nest", .difficulty=4.0f, .probability=0.3f,
	  .habitats=0x1,  // set as bits
	  .flags=f_Repeat, .members={
		{Creature::Doxy, 4,6},
	}},
};

static std::vector<int> s_num_spawned;

static Ragged<Creature::Handle> s_active_squads;

//-------------------------------------------------------------------------------------------------
// Interface

void init()
{
	s_num_spawned.reserve(s_squads.size());
	s_active_squads.resize(c_MaxActiveSquads);
	for (Creature::HandleList& row : s_active_squads)
	{
		row.reserve(c_MaxSquadSize);
	}

	// Some validation
	for (Squad::Definition const& squad : s_squads)
	{
		assert(Util::Size(squad.members) > 0);
		for (Member const& member : squad.members)
		{
			assert(member.min_num >= 0);
			assert(member.max_num >= 1);
			assert(member.max_num >= member.min_num);
			assert(Creature::is_valid_type(member.type));
		}
	}
}

void clear()
{
	Util::Fill(s_num_spawned, Util::Size(s_squads), 0);
	
	for (Creature::HandleList& row : s_active_squads)
	{
		row.clear();
	}
}

void serialize(ISerializer& s)
{
	// Num spawned:
	int const real_num = Util::Size(s_num_spawned);
	int num_srz = real_num;
	s.srz_int(num_srz);
	for (int i = 0;
		i < real_num && i < num_srz;
		++i)
	{
		s.srz_int(s_num_spawned[i]);
	}

	// Active squads:
	int max_active = c_MaxActiveSquads;
	s.srz_int(max_active);
	for (int i = 0;
		i < max_active && i < c_MaxActiveSquads;
		++i)
	{
		s.srz_vector(s_active_squads[i], "Active squad");
	}
}

//int num_defined()
//{
//	return Util::Size(s_squads);
//}

bool is_defined(int squad_id)
{
	return Util::IsValidIndex(s_squads, squad_id);
}

Squad::Definition const& read_definition(int squad_id)
{
	return s_squads.at(squad_id);
}

bool can_spawn(int squad_id, float target_difficulty, Creature::Habitat habitat)
{
	Squad::Definition const& squad = s_squads[squad_id];

	if (squad.probability <= 0.0f ||
		!Spawn::difficulty_in_range(squad.difficulty, target_difficulty))
	{
		return false;
	}

	if (habitat != Creature::Habitat::None &&
		!squad.habitats.test((size_t)habitat))
	{
		return false;
	}

	if (!Util::IsFlagSet(squad.flags, f_Repeat) &&
		s_num_spawned[squad_id] > 0)
	{
		return false;
	}

	for (Member const& member : squad.members)
	{
		if (!Gingerbread::can_spawn_identity(member.type, target_difficulty))
		{
			return false;
		}
	}

	return true;
}

void find_spawn_options (float target_difficulty, Spawn::OptionTempList& out_list,
	FloatTempList& out_weights, Creature::Habitat habitat)
{
	for (int i = 0; i < Util::Size(s_squads); ++i)
	{
		Squad::Definition const& squad = s_squads[i];

		if (!can_spawn(i, target_difficulty, habitat))
		{
			continue;
		}

		float const probability = squad.probability *
			Spawn::probability_factor(squad.difficulty, target_difficulty);

		if (probability > 0.0f)
		{
			out_list.emplace_back(Spawn::Option::Type::Squad, i);
			out_weights.push_back(probability);
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Active squads

int find_free_index()
{
	for (int i = 0; i < c_MaxActiveSquads; ++i)
	{
		if (s_active_squads[i].empty())
		{
			return i;
		}
	}

	return c_Invalid;
}

Creature::HandleList& get_squad (int index)
{
	return s_active_squads.at(index);
}

void add_creature (int squad_index, Creature::Handle creature)
{
	Creature::HandleList& squad = s_active_squads.at(squad_index);
	if (Check(Util::Size(squad) < c_MaxSquadSize))
	{
		squad.push_back(creature);
	}
}

void remove_creature (int squad_index, Creature::Handle creature)
{
	Creature::HandleList& squad = s_active_squads.at(squad_index);
	Util::RemoveFirstMatchingItem(squad, creature);

	// Dissolve the squad once it's down to a single creature.
	if (Util::Size(squad) == 1)
	{
		squad.at(0).remove_from_squad();
	}
}

} // namespace Squad
