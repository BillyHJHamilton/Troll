#pragma once

#include "Creature.h"
#include "Geometry.h"
#include "Line.h"
#include "Types.h"

#include <optional>
#include <string>

// A beam is a spell flying through the air.
// Or just about anything else flying through the air.  Like a bird.
// And for that matter, it's also used for melee attacks.

namespace Beam
{
	enum Type : int
	{
		Spell,
		Melee,
		Projectile,
	};

	// Beam flags
	uint constexpr f_CasterAimed	= 1 << 0;
	uint constexpr f_Aggressive		= 1 << 1;

	struct Data
	{
		Vec3 start_pos;
		Vec3 target_pos;
		Vec3 pos;
		Beam::Type type;
		Spell::EffectFunc effect_func;
		char const* noun;
		char const* colour;
		Creature::Handle caster;
		Creature::Handle intended_target;
		int codepoint;
		int trajectory; // index in the line cache
		int max_range;
		int base_accuracy;
		int cloud_accuracy_loss;
		Damage::Type damage_type;
		int damage;
		int spell_power;
		uint beam_flags;
		uint target_flags;
		bool done;
	};

	void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
		bool caster_aimed, int line_id);

	void shoot_ability (Ability::Index ability, Creature::Handle user, Vec3 target_pos,
		int line_id);

	int accuracy_at_range (int base_accuracy, Vec3 start, Vec3 end);
}

