#pragma once

#include "Types.h"
#include "Geometry.h"

#include <optional>

enum class TargetMode : byte
{
	Automatic,
	Manual
};

extern TargetMode g_target_mode;
extern Creature::Handle g_target_creature;
extern Vec3 g_target_pos;

char const * const c_target_colour = "darkest grey";

namespace Target
{
	void clear();
	void update();
	void cycle();
	void move(Vec2 dir);

	bool is_target(Creature::Handle creature);
	bool is_target(Vec3 const& global_pos);
	std::optional<Vec3> get_pos();
}
