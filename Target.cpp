#include "Target.h"

#include "Creature.h"
#include "Draw.h"
#include "Player.h"
#include "World.h"

#include <algorithm>
#include <cassert>

namespace Target
{

TargetMode s_target_mode;
Creature::Handle s_target_creature;
Vec3 s_target_pos;

void clear ()
{
	s_target_mode = TargetMode::Automatic;
	s_target_creature = Creature::None;
}

void update ()
{
	if (s_target_mode == TargetMode::Automatic)
	{
		if (s_target_creature != c_Invalid)
		{
			if (s_target_creature.visible())
			{
				// track target
				s_target_pos = s_target_creature.pos();
			}
			else
			{
				// lose target
				s_target_creature = c_Invalid;
			}
		}

		if (s_target_creature == c_Invalid)
		{
			// acquire target
			cycle();
		}
	}
	else if (s_target_mode == TargetMode::Manual)
	{
		if (s_target_pos.z != Player::pos().z &&
			!World::read().is_visible(s_target_pos))
		{
			cycle();
		}
	}
}

void cycle ()
{
	if (s_target_mode == TargetMode::Manual)
	{
		s_target_creature = Creature::creature_at_pos(s_target_pos);
		s_target_mode = TargetMode::Automatic;
	}
	
	std::vector<Creature::Handle> const & visible_creatures = Creature::get_visible_creatures();
	if (visible_creatures.size() > 0)
	{
		int vci = -1; // visible creature index - that is, index on the vector

		// see if the current target is in the list
		if (s_target_creature != -1)
		{
			auto itr = std::find(
				visible_creatures.begin(),
				visible_creatures.end(),
				s_target_creature);

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

		s_target_creature = visible_creatures[vci];
		s_target_pos = s_target_creature.pos();
	}
	else
	{
		s_target_creature = -1;
	}
}

void move (Vec2 dir)
{
	if (s_target_mode == TargetMode::Automatic)
	{
		if (s_target_creature == Creature::None)
		{
			s_target_pos = Player::pos();
		}
		s_target_mode = TargetMode::Manual;
	}
	s_target_pos += dir.xy0();

	// see if we've slipped onto another level
	Draw::View const& view = Draw::get_view();
	s_target_pos.z = view.z;
	for (Vec3 peek : view.peek_tiles)
	{
		if (peek.xy() == s_target_pos.xy())
		{
			s_target_pos.z = peek.z;
			break;
		}
	}
}

bool is_target (Creature::Handle creature)
{
	if (s_target_mode == TargetMode::Automatic)
	{
		return creature == s_target_creature;
	}
	else if (s_target_mode == TargetMode::Manual)
	{
		return creature.valid()
			&& creature.pos() == s_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

bool is_target (Vec3 const & global_pos)
{
	if (s_target_mode == TargetMode::Manual)
	{
		return global_pos == s_target_pos;
	}
	else if (s_target_mode == TargetMode::Automatic)
	{
		return s_target_creature.valid()
			&& s_target_pos == global_pos;
	}
	else
	{
		assert(false); // unhandled case
		return false;
	}
}

std::optional<Vec3> get_pos ()
{
	if (s_target_mode == TargetMode::Automatic)
	{
		if (s_target_creature.valid())
		{
			return s_target_creature.pos();
		}
		else
		{
			return std::optional<Vec3>(); // none
		}
	}
	else if (s_target_mode == TargetMode::Manual)
	{
		return s_target_pos;
	}
	else
	{
		assert(false); // unhandled case
		return Vec3 {0,0,0};
	}
}

}