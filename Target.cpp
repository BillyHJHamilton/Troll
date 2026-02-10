#include "Target.h"

#include "Creature.h"
#include "Draw.h"
#include "Player.h"
#include "World.h"

#include <algorithm>
#include <cassert>

namespace Target
{

TargetMode g_target_mode;
Creature::Handle g_target_creature;
Vec3 g_target_pos;

void clear ()
{
	g_target_mode = TargetMode::Automatic;
	g_target_creature = Creature::None;
}

void update ()
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_creature != c_invalid)
		{
			if (g_target_creature.visible())
			{
				// track target
				g_target_pos = g_target_creature.pos();
			}
			else
			{
				// lose target
				g_target_creature = c_invalid;
			}
		}

		if (g_target_creature == c_invalid)
		{
			// acquire target
			cycle();
		}
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		if (g_target_pos.z != Player::pos().z &&
			!World::read().is_visible(g_target_pos))
		{
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
	g_target_pos += dir.xy0();

	// see if we've slipped onto another level
	Draw::View const& view = Draw::get_view();
	g_target_pos.z = view.z;
	for (Vec3 peek : view.peek_tiles)
	{
		if (peek.xy() == g_target_pos.xy())
		{
			g_target_pos.z = peek.z;
			break;
		}
	}
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

bool is_target (Vec3 const & global_pos)
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

std::optional<Vec3> get_pos ()
{
	if (g_target_mode == TargetMode::Automatic)
	{
		if (g_target_creature.valid())
		{
			return g_target_creature.pos();
		}
		else
		{
			return std::optional<Vec3>(); // none
		}
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		return g_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return Vec3 {0,0,0};
	}
}

}