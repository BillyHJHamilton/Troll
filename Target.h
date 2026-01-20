#pragma once

#include "Geometry.h"

#include "Types.h"

#include <optional>

enum class TargetMode : int
{
	Automatic,
	Manual
};

extern TargetMode g_target_mode;
extern Creature::Handle g_target_creature;
extern Vec2 g_target_pos;

char const * const TARGET_COLOUR = "darkest red";

namespace Target
{
	void clear();
	void update();
	void cycle();
	void move(Vec2 dir);

	bool is_target(Creature::Handle creature);
	bool is_target(Vec2 const& global_pos);
	std::optional<Vec2> get_pos();
}