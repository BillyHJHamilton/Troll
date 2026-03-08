#pragma once

#include "Geometry.h"
#include "Types.h"

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
		Spell::Index miscast_spell = Spell::None;

		bool game_over = false;
		Creature::Type defeated_by = (Creature::Type)-1;

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
	Creature::Type get_defeated_by ();
	int current_level ();
	int current_xp ();
	int next_xp_threshold ();

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
	void set_game_over (Creature::Type instigator);
	void gain_xp_for (Creature::Type creature);
};
