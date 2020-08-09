#pragma once

#include "Geometry.h"

enum class TargetMode : int
{
	Automatic,
	Manual
};

extern TargetMode g_target_mode;
extern int g_target_index;
extern Vec2 g_target_pos;

void clear_target ();
void update_target ();
void cycle_target ();

bool creature_is_targeted(int creature_index);
