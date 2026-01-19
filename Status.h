#pragma once

#include "Types.h"

#include "Creature.h"

#include <string>

namespace Status
{
	using CalcFunc = void(*)(Creature::Handle creature, Creature::DerivedStats & ds, int severity);
	using EndRoundFunc = void(*)(Creature::Handle creature);
	using CureFunc = void(*)(Creature::Handle const creature);

	enum Index : int
	{
		Shield = 0,
		Dancing,
		Tickled,
		TongueTied,

		Count
	};

	struct Data
	{
		char const * abbrev;

		// function pointers
		CalcFunc calc_func;
		EndRoundFunc end_round_func;
		CureFunc cure_func;
	};

	void init();

	int max_severity(Status::Index status);
	std::string abbrev(Status::Index status);

	void apply_to_derived_stats (Status::Index status, Creature::Handle creature_index,
		Creature::DerivedStats & derived_stats);

	void do_endround(Creature::Handle creature);

	void print_cure_message(Creature::Handle const creature, Status::Index status);
};
