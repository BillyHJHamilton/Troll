#include "Target.h"

#include "Creature.h"
#include "Player.h"


#include <algorithm>
#include <cassert>

TargetMode g_target_mode;
int g_target_index;
Vec2 g_target_pos;

void clear_target ()
{
	g_target_mode = TargetMode::Automatic;
	g_target_index = -1;
}

void update_target ()
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_index != -1)
		{
			if (creature_visible(g_target_index))
			{
				// track target
				g_target_pos = creature_pos(g_target_index);
			}
			else
			{
				// lose target
				g_target_index = -1;
			}
		}

		if (g_target_index == -1)
		{
			// acquire target
			cycle_target();
		}
	}
}

void cycle_target ()
{
	if (g_target_mode == TargetMode::Manual)
	{
		g_target_index = creature_at_pos(g_target_pos);
		g_target_mode = TargetMode::Automatic;
	}
	
	if (g_visible_creatures.size() > 0)
	{
		int vci = -1; // visible creature index - that is, index on the vector

		// see if the current target is in the list
		if (g_target_index != -1)
		{
			auto itr = std::find(
				g_visible_creatures.begin(),
				g_visible_creatures.end(),
				g_target_index);

			if (itr != g_visible_creatures.end())
			{
				vci = static_cast<int>(itr - g_visible_creatures.begin());
			}
		}

		// cycle to next target
		++ vci;

		// wrap around
		if (vci >= g_visible_creatures.size())
		{
			vci = 0;
		}

		g_target_index = g_visible_creatures[vci];
		g_target_pos = creature_pos(g_target_index);
	}
	else
	{
		g_target_index = -1;
	}
}

void move_target_pos (Vec2 dir)
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_index == Creature::None)
		{
			g_target_pos = Player::pos();
		}
		g_target_mode = TargetMode::Manual;
	}
	g_target_pos += dir;
}

bool creature_is_targeted (int creature_index)
{
	if (g_target_mode == TargetMode::Automatic)
	{
		return creature_index == g_target_index;
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		return creature_valid(creature_index)
			&& creature_pos(creature_index) == g_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

bool pos_is_targeted (Vec2 const & global_pos)
{
	if (g_target_mode == TargetMode::Manual)
	{
		return global_pos == g_target_pos;
	}
	else if (g_target_mode == TargetMode::Automatic)
	{
		return creature_valid(g_target_index)
			&& g_target_pos == global_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

