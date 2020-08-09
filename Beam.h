#pragma once

#include "Types.h"
#include "Geometry.h"

#include <string>

// A beam is a spell flying through the air.
// Or just about anything else flying through the air.  Like a bird.

namespace Beam
{
	enum Type : int
	{
		Spell,
		Projectile
	};

	struct Data
	{
		Vec2 pos;
		Vec2 trajectory; // may be any non-zero length
		Beam::Type type;
		int caster;
		bool caster_aimed;
		int intended_target;
		bool done;
	};

	void shoot_spell (Spell::Index spell, int caster, Vec2 target_pos, bool caster_aimed);
}

/*
struct Beam2
{
	// Data
	int damage;
	int accuracy;

	Spell::Index spell_index; // if any
	Spell::EffectFunc effect_func;
	int caster_index; // if any
	bool caster_aimed;

	std::string description;
	int codepoint;
	std::string colour;

	Vec2 start_pos;
	Vec2 target_pos;
	bool done;

	// Functions

	static Beam make_spell (Spell::Index spell, int caster);
	// make fake spell, projectile...

	static void shoot_at_target (Beam & beam, int caster, int target);
	static void shoot_at_pos (Beam & beam, int caster, Vec2 target_pos);
	static void shoot_along_line (Beam & beam);
	
	static int get_hit_chance (Beam const & beam, int caster, int target);
	static void hit_creature (Beam const & beam, int caster, int target);
};
*/

