#include "Status.h"

#include "Creature.h"

#include <vector>

namespace Status
{

// functions for calculating stat effects of each status.
static void calculate_dancing(int creature, Creature::DerivedStats & ds, int severity);


Status::Data s_status_data [Status::Count];

void init ()
{
	s_status_data[Status::Shield] = {"Shield", nullptr, nullptr};
	s_status_data[Status::Dancing] = {"Dance", &calculate_dancing, nullptr};
	s_status_data[Status::Tickled] = {"Tickle", nullptr, nullptr};
	s_status_data[Status::TongueTied] = {"TngTie", nullptr, nullptr};
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

void apply_to_derived_stats (Status::Index status, int creature_index,
	Creature::DerivedStats & derived_stats)
{
	Status::Data sd = s_status_data[status];
	if (sd.calc_func != nullptr)
	{
		int severity = creature_status_severity(creature_index, status);
		sd.calc_func(creature_index, derived_stats, severity);
	}
}


// ------------------------------------------------------------------------------------------------
// Note to self - Miscastiness in this game is much more potent than in HPADS (we removed a factor
// of 0.4 elsewhere in the calculations).  Therefore, if the old effect was +5 miscastiness,
// instead add only +2 to achieve the same result.
// ------------------------------------------------------------------------------------------------



void calculate_dancing(int creature, Creature::DerivedStats & ds, int severity)
{
	ds.distractedness += 12*severity;
}


} // namespace status
