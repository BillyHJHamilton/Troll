#pragma once

#include "Geometry.h"

#include <optional>

enum class TargetMode : int
{
	Automatic,
	Manual
};

extern TargetMode g_target_mode;
extern int g_target_index;
extern Vec2 g_target_pos;

char const * const TARGET_COLOUR = "darkest red";

void clear_target ();
void update_target ();
void cycle_target ();
void move_target_pos (Vec2 dir);

bool creature_is_targeted (int creature_index);
bool pos_is_targeted (Vec2 const & global_pos);
std::optional<Vec2> get_target_pos ();
