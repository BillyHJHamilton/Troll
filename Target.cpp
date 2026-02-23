#include "Target.h"

#include "Codepoint.h"
#include "Colour.h"
#include "Creature.h"
#include "Draw.h"
#include "Player.h"
#include "Terrain.h"
#include "Visibility.h"
#include "World.h"

#include <algorithm>
#include <cassert>

char const* g_TargetColour;

namespace Target
{

TargetMode s_target_mode;
Creature::Handle s_target_creature;
Vec3 s_target_pos;

void init ()
{
	g_TargetColour = cstr_DarkestGrey;
	//g_TargetColour = cstr_DarkerViolet;
}

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
		if (!Draw::get_view().contains_global_pos(s_target_pos))
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

	Draw::View const& view = Draw::get_view();

	Vec3 new_pos = s_target_pos + dir.xy0();

	// see if we've slipped onto another level
	new_pos.z = view.get_z(new_pos.xy());

	if (view.contains_global_pos(new_pos))
	{
		s_target_pos = new_pos;
	}
}

bool is_valid()
{
	if (s_target_mode == TargetMode::Automatic)
	{
		return s_target_creature.valid();
	}
	else if (s_target_mode == TargetMode::Manual)
	{
		return true;
	}
	else
	{
		assert(false); // unhandled case
		return false;
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

char const* colour()
{
	return colour(Visibility::Visible, false);
}

char const* colour(Visibility visibility, bool is_wall)
{
	if (is_wall)
	{
		if (visibility == Visibility::Visible)
		{
			return cstr_DarkWhite;
		}
		else if (visibility == Visibility::Explored)
		{
			return cstr_DarkerGrey;
		}
	}

	return cstr_DarkestGrey;
}

void draw (Draw::View view)
{
	if (Target::is_valid())
	{
		Vec3 const pos = get_pos().value();
		if (view.contains_global_pos(pos))
		{
			Visibility const vis = World::read().get_visibility(pos);
			bool const is_wall = World::read().get_terrain(pos) == Terrain::Wall;

			bool draw = true;
			char const* draw_colour = Target::colour(vis, is_wall);
			int codepoint = Codepoint::OpenCursor;

			if (vis == Visibility::Visible)
			{
				draw = is_wall;
				codepoint = Codepoint::SolidBlock;
			}

			if (draw)
			{
				Draw::TerminalLayer layer(Draw::TerminalLayer::Cursor);
				Draw::draw_tile(codepoint, pos.xy(), view, draw_colour);
			}
		}
	}
}

} // namespace Target
