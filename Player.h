#pragma once

#include "Types.h"

#include "Damage.h"
#include "Geometry.h"

namespace Player
{
	static int constexpr c_MaxNameLength = 16;
	static int constexpr c_VisionRadius = 8;

	enum class AutomoveType
	{
		None,
		Compass,
		Path,
		Explore,
	};

	struct Data
	{
		std::string name;

		AutomoveType automove_type = AutomoveType::None;
		CompassDirection automove_dir = c_CompassInvalid;
		bool acted = false;

		float sugar = 65.0f; // 0 - 100

		// Tracking when player last miscasted a spell.
		// Used to trigger certain taunts.
		int miscast_turn = -1;
		Spell::Index miscast_spell = (Spell::Index)c_Invalid;

		Score::Ending ending = (Score::Ending)c_Invalid;
		Damage::Cause defeated_by = {};

		int level = 1;
		int xp = 0;

		void serialize(ISerializer& s);
	};

	// Initialization
	void clear();
	void serialize(ISerializer& s);

	// Getters
	Vec3 pos ();
	Creature::Handle handle ();
	const std::string& name ();
	bool is_automoving ();
	bool has_acted ();
	float get_sugar ();
	int get_sugar_bonus ();
	char const* get_sugar_colour ();
	Spell::Index get_recent_miscast ();
	bool is_game_over ();
	Score::Ending get_ending ();
	Damage::Cause get_defeated_by ();
	int current_level ();
	int current_xp ();
	int next_xp_threshold ();
	int total_xp_spent ();

	// Mutators
	void set_name (std::string str);
	void start_automove (CompassDirection dir);
	void start_pathfind (Vec3 target);
	void auto_collect ();
	void auto_darkness ();
	void auto_explore ();
	void stop_automove ();
	void dispatch_automove ();
	void set_acted (bool acted);
	void tick_sugar ();
	void gain_sugar (int amount);
	void set_miscasted (Spell::Index spell_index);
	void set_defeated (Damage::Cause defeated_by);
	void set_won ();
	void gain_xp_for (Creature::Type creature);
};
