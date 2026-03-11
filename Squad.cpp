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

static std::vector<Squad::Data> const s_squads =
{
	{ .debug_name="Gnome Squad", .difficulty=1.0f, .probability=0.4f,
	  .flags=f_Repeat, .members={
		{Creature::Gnome, 2,5},
	}},

	{ .debug_name="Crabbe and Goyle", .difficulty=3.0f, .probability=1.0f,
	  .flags=f_None, .members={
		{Creature::Crabbe_3},
		{Creature::Goyle_3},
	}},

	{ .debug_name="Crab Squad", .difficulty=3.0f, .probability=0.3f,
	  .flags=f_Repeat, .members={
		{Creature::BigFireCrab},
		{Creature::FireCrab, 2,3},
	}},
};

static std::vector<int> s_num_spawned;

//-------------------------------------------------------------------------------------------------
// Interface

void init()
{
	s_num_spawned.reserve(s_squads.size());
}

void clear()
{
	Util::Fill(s_num_spawned, Util::Size(s_squads), 0);
}

void serialize(ISerializer& s)
{
	int const real_num = Util::Size(s_num_spawned);
	int num_srz = real_num;
	s.srz_int(num_srz);

	for (int i = 0;
		i < real_num && i < num_srz;
		++i)
	{
		s.srz_int(s_num_spawned[i]);
	}
}

int get_num()
{
	return Util::Size(s_squads);
}

bool can_spawn(int squad_id, float target_difficulty)
{
	Squad::Data const& squad = s_squads[squad_id];

	if (squad.probability <= 0.0f ||
		Spawn::difficulty_in_range(squad.difficulty, target_difficulty))
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

void find_spawn_options (float target_difficulty, Spawn::OptionTempList out_list,
	FloatTempList& out_weights)
{
	for (int i = 0; i < Util::Size(s_squads); ++i)
	{
		Squad::Data const& squad = s_squads[i];

		if (!can_spawn(i, target_difficulty))
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

} // namespace Squad
