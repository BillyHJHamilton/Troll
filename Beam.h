#pragma once

#include "Geometry.h"
#include "Line.h"
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
		Beam::Type type;
		Creature::Handle caster;
		Creature::Handle intended_target;
		int trajectory; // index in the line cache
		int max_range;
		bool caster_aimed;
		bool done;
	};

	void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec2 target_pos, bool caster_aimed);

	int accuracy_at_range (int base_accuracy, Vec2 start, Vec2 end);

	std::optional<LineCache::Itr> get_latest_impact_line ();
}

