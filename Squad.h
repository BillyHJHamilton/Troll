#pragma once

#include "Types.h"
#include "BitFlag.h"
#include "Creature.h"
#include "Spawn.h"

// Code related to spawning groups of several creatures at once (a squad).
namespace Squad
{
	int constexpr c_MaxSquadSize = 10;
	int constexpr c_MaxActiveSquads = 100;

	// Squad flags
	uint constexpr f_Repeat		= 1 << 0;

	struct Member
	{
		Creature::Type type = (Creature::Type)c_Invalid;
		int min_num = 1;
		int max_num = 1;
	};
	using MemberList = std::vector<Member>;

	struct Definition
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

	//---------------------------------------------------------------------------------------------
	// Spawning squads

	//int num_defined();
	bool is_defined(int squad_id);
	Squad::Definition const& read_definition(int squad_id);
	bool can_spawn(int squad_id, float target_difficulty);
	void find_spawn_options (float target_difficulty, Spawn::OptionTempList& out_list,
		FloatTempList& out_weights);

	//---------------------------------------------------------------------------------------------
	// Active squads

	int find_free_index ();
	Creature::HandleList& get_squad (int index);
	void add_creature (int squad_index, Creature::Handle creature);
	void remove_creature (int squad_index, Creature::Handle creature);
//	int add_active_squad(Creature::HandleTempList const& handles);
};
