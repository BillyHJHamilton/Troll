#include "Status.h"

#include "Creature.h"
#include "Draw.h"
#include "Geometry.h"
#include "Grammar.h"
#include "Random.h"

#include <vector>

namespace Status
{

// functions for calculating stat effects of each status.
void calc_dancing(Creature::Handle creature, Creature::DerivedStats & ds, int severity);

// functions for updating each status at end of round
void endround_dancing(Creature::Handle creature);

// functions for printing message when a status is removed
void cure_dancing(Creature::Handle const creature);

Status::Data s_status_data [Status::Count];

void init ()
{
	s_status_data[Status::Shield] = {"Shield", nullptr, nullptr, nullptr};
	s_status_data[Status::Dancing] = {"Dance", &calc_dancing, &endround_dancing, &cure_dancing};
	s_status_data[Status::Tickled] = {"Tickle", nullptr, nullptr, nullptr};
	s_status_data[Status::TongueTied] = {"TngTie", nullptr, nullptr, nullptr};
}

int max_severity (Status::Index status_index)
{
	if (status_index == Status::Shield)
		return 4;
//	else if (status_index == Status::Disintegration)
//		return 2;
//	else if (status_index == Status::Hate)
//		return 100;
	else
		return 10;
}

std::string abbrev(Status::Index status)
{
	return s_status_data[status].abbrev;
}

void apply_to_derived_stats (Status::Index status, Creature::Handle creature,
	Creature::DerivedStats & derived_stats)
{
	Status::Data sd = s_status_data[status];
	if (sd.calc_func != nullptr)
	{
		int severity = creature.status_severity(status);
		sd.calc_func(creature, derived_stats, severity);
	}
}

void do_endround(Creature::Handle creature)
{
	for (int status = 0; status < Status::Count; ++status)
	{
		EndRoundFunc func = s_status_data[status].end_round_func;
		if (creature.has_status((Index)status) && func != nullptr)
		{
			func(creature);
		}
	}
}

void print_cure_message(Creature::Handle const creature, Status::Index status)
{
	CureFunc cure_func = s_status_data[status].cure_func;
	if (cure_func != nullptr)
	{
		cure_func(creature);
	}
}

// ------------------------------------------------------------------------------------------------
// Note to self - Miscastiness in this game is much more potent than in HPADS (we removed a factor
// of 0.4 elsewhere in the calculations).  Therefore, if the old effect was +5 miscastiness,
// instead add only +2 to achieve the same result.
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// Dancing

void calc_dancing(Creature::Handle creature, Creature::DerivedStats & ds, int severity)
{
	ds.distractedness += 12*severity;
}

void endround_dancing(Creature::Handle creature)
{
	// Random movement
	if (random(0, 2 + creature.status_severity(Dancing) > 2))
	{
		Vec2 move_dir = { random(-1,1), random(-1,1) };
		if (move_dir != Vec2{0,0})
		{
			creature.try_move(move_dir);
		}
	}

	creature.reduce_status(Dancing, 1);
}

void cure_dancing(Creature::Handle const creature)
{
	add_game_message(Grammar::Your(creature) + " feet stop dancing.");
}

} // namespace status
