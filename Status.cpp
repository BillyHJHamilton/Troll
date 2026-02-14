#include "Status.h"

#include "Action.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Geometry.h"
#include "Grammar.h"
#include "Pathfind.h"
#include "Random.h"
#include "Terrain.h"
#include "World.h"

#include <format>
#include <vector>

namespace Status
{

// functions for calculating stat effects of each status.
void calc_dancing(Creature::Handle creature, Creature::DerivedStats & ds, int severity);
void calc_leg_locked(Creature::Handle creature, Creature::DerivedStats& ds, int severity);
void calc_tickled(Creature::Handle creature, Creature::DerivedStats& ds, int severity);
void calc_tongue_tied(Creature::Handle creature, Creature::DerivedStats& ds, int severity);
void calc_burning(Creature::Handle creature, Creature::DerivedStats& ds, int severity);
void calc_impeded(Creature::Handle creature, Creature::DerivedStats& ds, int severity);
void calc_batty(Creature::Handle creature, Creature::DerivedStats& ds, int severity);

// functions for updating each status at end of round
void endround_dancing(Creature::Handle creature);
void endround_leg_locked(Creature::Handle creature);
void endround_tickled(Creature::Handle creature);
void endround_tongue_tied(Creature::Handle creature);
void endround_burning(Creature::Handle creature);
void endround_impeded(Creature::Handle creature);
void endround_batty(Creature::Handle creature);

// functions for printing message when a status is removed
void cure_dancing(Creature::Handle const creature);
void cure_leg_locked(Creature::Handle const creature);
void cure_tickled(Creature::Handle const creature);
void cure_tongue_tied(Creature::Handle const creature);
void cure_burning(Creature::Handle const creature);
void cure_impeded(Creature::Handle const creature);
void cure_batty(Creature::Handle const creature);

Status::Data s_status_data [Status::Count];

void init ()
{
	s_status_data[Status::Shield] = {"Shield", nullptr, nullptr, nullptr};
	s_status_data[Status::Dancing] = {"Dance", &calc_dancing, &endround_dancing, &cure_dancing};
	s_status_data[Status::LegLocked] = { "LegLk", &calc_leg_locked, &endround_leg_locked, &cure_leg_locked };
	s_status_data[Status::Tickled] = {"Tickle", &calc_tickled, &endround_tickled, &cure_tickled};
	s_status_data[Status::TongueTied] = {"TngTie", &calc_tongue_tied, &endround_tongue_tied, &cure_tongue_tied};
	s_status_data[Status::Burning] = {"Fire", &calc_burning, &endround_burning, &cure_burning};
	s_status_data[Status::Impeded] = {"Imped", &calc_impeded, &endround_impeded, &cure_impeded};
	s_status_data[Status::Batty] = {"Batty", &calc_batty, &endround_batty, &cure_batty};
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
	// Chance of random movement.
	int const roll_max = 2 + creature.status_severity(Dancing);
	int const roll = Random::in_range(0, roll_max);

	if (c_ShowActionDebug)
	{
		std::cout << std::format("{} dances on 3+.  Roll (0-{}) = {}\n",
			creature.short_name(), roll_max, roll);
	}

	if (roll >= 3)
	{
		Vec3 const old_pos = creature.pos();

		std::vector<Vec3> possible_moves;
		Pathfind::find_open_neighbours(old_pos, possible_moves, Creature::None);
		Vec3 const move_to = Random::from_vector(possible_moves);
		Vec2 const step = move_to.xy() - old_pos.xy();

		bool const moved = try_move(creature, step, MoveMode::Forced);
		Vec3 const new_pos = creature.pos();

		// Check for falling down stairs
		if (moved && new_pos.z < old_pos.z &&
			World::read().get_terrain(new_pos) == Terrain::UpStairs)
		{
			creature.take_damage(3, Creature::None);

			if (World::read().is_visible(old_pos) ||
				World::read().is_visible(new_pos))
			{
				Draw::add_message(Grammar::You(creature) + " " +
					Grammar::verbs("fall", creature) + " down the stairs!");
			}
		}
	}

	creature.reduce_status(Dancing, 1);
}

void cure_dancing(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::Your(creature) + " feet stop dancing.");
}

// ------------------------------------------------------------------------------------------------
// Leg Locked

void calc_leg_locked(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	ds.evasion -= 25;
	ds.walk_failure += 15 + (8 * severity);
}

void endround_leg_locked(Creature::Handle creature)
{
	creature.reduce_status(LegLocked, 1);
}

void cure_leg_locked(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::Your(creature) + " legs are no longer locked together.");
}

// ------------------------------------------------------------------------------------------------
// Tickled

void calc_tickled(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	ds.distractedness += 4*severity;
	ds.miscastiness += 5*severity;
}

void endround_tickled(Creature::Handle creature)
{
	creature.reduce_status(Tickled, 1);
}

void cure_tickled(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::You_are(creature) + " no longer being tickled.");
}

// ------------------------------------------------------------------------------------------------
// Tongue Tied

void calc_tongue_tied(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	ds.miscastiness += 9*severity;
}

void endround_tongue_tied(Creature::Handle creature)
{
	creature.reduce_status(TongueTied, 1);
}

void cure_tongue_tied(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::You_are(creature) + " no longer tongue-tied.");
}

// ------------------------------------------------------------------------------------------------
// Burning

void calc_burning(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	ds.distractedness += 40;
}

void endround_burning(Creature::Handle creature)
{
	creature.reduce_status(Burning, 1);
	if (creature.has_status(Burning))
	{
		Draw::creature_message(creature, Grammar::You_are(creature) + " burned!");
		creature.take_damage(1, Creature::None);
	}
}

void cure_burning(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::Your(creature) + " clothes have gone out.");
}

// ------------------------------------------------------------------------------------------------
// Impeded

void calc_impeded(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	//ds. += 40;
	ds.walk_failure += 12*severity;
	ds.evasion -= 5*severity;
}

void endround_impeded(Creature::Handle creature)
{
	creature.reduce_status(Impeded, 1);
}

void cure_impeded(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::Your(creature) + " movement is no longer impeded.");
}

// ------------------------------------------------------------------------------------------------
// Batty

void calc_batty(Creature::Handle creature, Creature::DerivedStats& ds, int severity)
{
	ds.distractedness += 10*severity;
	ds.evasion += 10*severity;
}

void endround_batty(Creature::Handle creature)
{
	creature.reduce_status(Batty, 1);
}

void cure_batty(Creature::Handle const creature)
{
	Draw::creature_message(creature, Grammar::You_are(creature)
		+ " no longer being attacked by black winged things.");
}


} // namespace status
