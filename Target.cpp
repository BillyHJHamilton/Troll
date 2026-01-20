#include "Target.h"

#include "Creature.h"
#include "Player.h"

#include <algorithm>
#include <cassert>

namespace Target
{

TargetMode g_target_mode;
Creature::Handle g_target_creature;
Vec2 g_target_pos;

void clear ()
{
	g_target_mode = TargetMode::Automatic;
	g_target_creature = Creature::None;
}

void update ()
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_creature != -1)
		{
			if (g_target_creature.visible())
			{
				// track target
				g_target_pos = g_target_creature.pos();
			}
			else
			{
				// lose target
				g_target_creature = -1;
			}
		}

		if (g_target_creature == -1)
		{
			// acquire target
			cycle();
		}
	}
}

void cycle ()
{
	if (g_target_mode == TargetMode::Manual)
	{
		g_target_creature = Creature::creature_at_pos(g_target_pos);
		g_target_mode = TargetMode::Automatic;
	}
	
	std::vector<Creature::Handle> const & visible_creatures = Creature::get_visible_creatures();
	if (visible_creatures.size() > 0)
	{
		int vci = -1; // visible creature index - that is, index on the vector

		// see if the current target is in the list
		if (g_target_creature != -1)
		{
			auto itr = std::find(
				visible_creatures.begin(),
				visible_creatures.end(),
				g_target_creature);

			if (itr != visible_creatures.end())
			{
				vci = static_cast<int>(itr - visible_creatures.begin());
			}
		}

		// cycle to next target
		++ vci;

		// wrap around
		if (vci >= visible_creatures.size())
		{
			vci = 0;
		}

		g_target_creature = visible_creatures[vci];
		g_target_pos = g_target_creature.pos();
	}
	else
	{
		g_target_creature = -1;
	}
}

void move (Vec2 dir)
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_creature == Creature::None)
		{
			g_target_pos = Player::pos();
		}
		g_target_mode = TargetMode::Manual;
	}
	g_target_pos += dir;
}

bool is_target (Creature::Handle creature)
{
	if (g_target_mode == TargetMode::Automatic)
	{
		return creature == g_target_creature;
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		return creature.valid()
			&& creature.pos() == g_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

bool is_target (Vec2 const & global_pos)
{
	if (g_target_mode == TargetMode::Manual)
	{
		return global_pos == g_target_pos;
	}
	else if (g_target_mode == TargetMode::Automatic)
	{
		return g_target_creature.valid()
			&& g_target_pos == global_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

std::optional<Vec2> get_pos ()
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_creature.valid())
		{
			return g_target_creature.pos();
		}
		else
		{
			return std::optional<Vec2>(); // none
		}
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		return g_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return Vec2 {0,0};
	}
}

}