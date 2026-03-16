#pragma once

#include "Types.h"

#include "Damage.h"
#include "Geometry.h"

namespace Player
{
	static int constexpr vision_radius = 8;

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

		// Tracking when player last miscasted a spell.
		// Used to trigger certain taunts.
		int miscast_turn = -1;
		Spell::Index miscast_spell = (Spell::Index)c_Invalid;

		bool game_over = false;
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
	Spell::Index get_recent_miscast ();
	bool is_game_over ();
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
	void set_miscasted (Spell::Index spell_index);
	void set_game_over (Damage::Cause defeated_by);
	void gain_xp_for (Creature::Type creature);
};
