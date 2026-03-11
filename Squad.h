#pragma once

#include "Types.h"
#include "BitFlag.h"
#include "Spawn.h"

// Code related to spawning groups of several creatures at once (a squad).
namespace Squad
{
	// Squad flags
	uint constexpr f_Repeat		= 1 << 0;

	struct Member
	{
		Creature::Type type = (Creature::Type)c_Invalid;
		int min_num = 1;
		int max_num = 1;
	};
	using MemberList = std::vector<Member>;

	struct Data
	{
		char const* debug_name = "Error Squad";
		float difficulty = 0.0f;
		float probability = 0.0f;
		uint flags = f_None;
		MemberList members;
	};

	void init();
	void clear();
	void serialize(ISerializer& s);

	int get_num();
	bool is_valid_id(int squad_id);
	Squad::Data const& read_data(int squad_id);
	bool can_spawn(int squad_id, float target_difficulty);
	void find_spawn_options (float target_difficulty, Spawn::OptionTempList out_list,
		FloatTempList& out_weights);
};
