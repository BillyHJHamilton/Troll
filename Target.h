#pragma once

#include "Types.h"
#include "Geometry.h"

#include <optional>

enum class TargetMode : byte
{
	Automatic,
	Manual
};

extern char const * g_TargetColour;

namespace Target
{
	void init();
	void clear();
	void update();
	void cycle();
	void move(Vec2 dir);

	bool is_target(Creature::Handle creature);
	bool is_target(Vec3 const& global_pos);
	std::optional<Vec3> get_pos();
}
