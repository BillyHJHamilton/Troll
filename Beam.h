#pragma once

#include "Geometry.h"
#include "Types.h"
#include "Creature.h"

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

	struct Data
	{
		Vec2 start_pos;
		Vec2 pos;
		Vec2 trajectory; // may be any non-zero length
		Beam::Type type;
		Creature::Handle caster;
		int max_range;
		int intended_target;
		bool caster_aimed;
		bool done;
	};

	void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec2 target_pos, bool caster_aimed);

	int accuracy_at_range (int base_accuracy, Vec2 start, Vec2 end);

	std::optional<LineItr> get_latest_impact_line ();
}

