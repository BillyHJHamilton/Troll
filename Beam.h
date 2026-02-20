#pragma once

#include "Creature.h"
#include "Geometry.h"
#include "Line.h"
#include "Types.h"

#include <optional>
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

	// Beam flags
	uint constexpr f_CasterAimed   = 1 << 0;
	uint constexpr f_StopOnTarget = 1 << 2;

	struct Data
	{
		Vec3 start_pos;
		Vec3 target_pos;
		Vec3 pos;
		Beam::Type type;
		Creature::Handle caster;
		Creature::Handle intended_target;
		int trajectory; // index in the line cache
		int max_range;
		int cloud_accuracy_loss;
		uint flags;
		bool done;
	};

	void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
		bool caster_aimed, int line_id);

	int accuracy_at_range (int base_accuracy, Vec3 start, Vec3 end);
}

