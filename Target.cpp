#include "Target.h"

#include "Codepoint.h"
#include "Colour.h"
#include "Config.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Player.h"
#include "Scratch.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "Visibility.h"
#include "World.h"

#include <algorithm>
#include <cassert>

namespace Target
{

enum class TargetMode : byte
{
	Automatic_Creature, // or no target, if s_target_creature is invalid
	Automatic_Position,
	Manual
};

TargetMode s_target_mode;
Creature::Handle s_target_creature;
Vec3 s_target_pos;
bool s_manual_automatic = false; // true if player manually cycled onto current creature

//-------------------------------------------------------------------------------------------------
// Helper functions

bool is_feature_target(Vec3 pos)
{
	return Terrain::is_spell_target(World::read().get_terrain(pos));
}

void find_visible_features(std::vector<Vec3,Scratch<Vec3>>& out_targets)
{
	Vec2 p = Player::pos().xy();
	int z = Player::pos().z;
	int r = Player::c_VisionRadius;

	// todo We could create a "LOS iterator" to encapsulate this.
	Box2 vis_area = Box2::around_tile(p, r);
	for (BoxItr itr(vis_area); itr; ++itr)
	{
		Vec3 p3 = itr->xyz(z);
		if (World::read().is_visible(p3) &&
			!Creature::creature_at_pos(p3).valid() &&
			is_feature_target(p3))
		{
			out_targets.push_back(p3);
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Interface functions

void clear ()
{
	s_target_mode = TargetMode::Automatic_Creature;
	s_target_creature.invalidate();
	s_manual_automatic = false;
}

void update ()
{
	if (s_target_mode == TargetMode::Automatic_Creature)
	{
		if (s_target_creature.valid())
		{
			if (s_target_creature.visible())
			{
				// track target
				s_target_pos = s_target_creature.pos();
			}
			else
			{
				// lose target
				s_target_creature.invalidate();
			}
		}

		if (!s_target_creature.valid())
		{
			// acquire target
			cycle(1, /*manually*/ false);
		}
	}

	else if (s_target_mode == TargetMode::Automatic_Position)
	{
		if (!World::read().is_visible(s_target_pos) ||
			!is_feature_target(s_target_pos) ||
			(Creature::has_visible_enemy() && !s_manual_automatic))
		{
			cycle(1, /*manually*/ false);
		}
	}

	else if (s_target_mode == TargetMode::Manual)
	{
		if (!Draw::get_view().contains_global_pos(s_target_pos))
		{
			cycle(1, /*manually*/ false);
		}
	}
}

void cycle (int step, bool manually)
{
	// If we selected this manually, don't automatically switch off.
	s_manual_automatic = manually;

	if (s_target_mode == TargetMode::Manual)
	{
		s_target_creature = Creature::creature_at_pos(s_target_pos);
		if (s_target_creature.valid())
		{
			s_target_mode = TargetMode::Automatic_Creature;
		}
		else
		{
			s_target_mode = TargetMode::Automatic_Position;
		}
	}

	Creature::HandleList const & visible_creatures = Creature::get_visible_creatures();

	// list of possible targets, with creatures followed by features
	std::vector<Vec3,Scratch<Vec3>> targets;
	targets.reserve(Util::Size(visible_creatures) + 10);
	for (Creature::Handle creature : visible_creatures)
	{
		targets.push_back(creature.pos());
	}

	// Don't consider features when in pure automatic mode, unless there are no creatures.
	if (targets.empty() || manually)
	{
		find_visible_features(targets);
	}

	int const num_targets = Util::Size(targets);
	if (targets.size() > 0)
	{
		// see if the current target is in the list
		int target_index = Util::FindIndex(targets, s_target_pos);

		if (target_index == c_Invalid)
		{
			target_index = 0;
		}
		else
		{
			// cycle to next target
			target_index += step;
		}

		// wrap around
		target_index = (num_targets + target_index) % num_targets;

		if (target_index <= Util::LastIndex(visible_creatures))
		{
			s_target_mode = TargetMode::Automatic_Creature;
			s_target_creature = visible_creatures[target_index];
			s_target_pos = s_target_creature.pos();
		}
		else
		{
			s_target_mode = TargetMode::Automatic_Position;
			s_target_creature.invalidate();
			s_target_pos = targets.at(target_index);
		}
	}
	else
	{
		s_target_mode = TargetMode::Automatic_Creature;
		s_target_creature.invalidate();
	}
}

void move (Vec2 dir)
{
	if (s_target_mode == TargetMode::Automatic_Creature &&
		!s_target_creature.valid())
	{
		if (s_target_creature == Creature::None)
		{
			s_target_pos = Player::pos();
		}
	}
	s_target_mode = TargetMode::Manual;

	Draw::View const& view = Draw::get_view();

	Vec3 new_pos = s_target_pos + dir.xy0();

	// see if we've slipped onto another level
	new_pos.z = view.get_z(new_pos.xy());

	if (view.contains_global_pos(new_pos))
	{
		s_target_pos = new_pos;
	}
}

void set_to(Vec3 new_pos)
{
	if (Draw::get_view().contains_global_pos(new_pos))
	{
		Creature::Handle creature = Creature::creature_at_pos(new_pos);
		if (creature.valid())
		{
			s_target_mode = TargetMode::Automatic_Creature;
			s_target_creature = creature;
			s_manual_automatic = true;
		}
		else if (is_feature_target(new_pos))
		{
			s_target_mode = TargetMode::Automatic_Position;
			s_manual_automatic = true;
		}
		else
		{
			s_target_mode = TargetMode::Manual;
		}

		s_target_pos = new_pos;
	}
}

void snap_to_player()
{
	s_target_mode = TargetMode::Automatic_Creature;
	s_target_creature = Player::handle();
	s_target_pos = Player::pos();
	s_manual_automatic = true;
}

bool is_valid()
{
	if (s_target_mode == TargetMode::Automatic_Creature)
	{
		return s_target_creature.valid();
	}
	else
	{
		return true;
	}
}

bool is_target (Creature::Handle creature)
{
	if (s_target_mode == TargetMode::Automatic_Creature)
	{
		return creature == s_target_creature;
	}
	else if (s_target_mode == TargetMode::Manual ||
		s_target_mode == TargetMode::Automatic_Position)
	{
		return creature.valid()
			&& creature.pos() == s_target_pos;
	}
	else
	{
		DebugBreak(); // unhandled case
		return false;
	}
}

bool is_target (Vec3 global_pos)
{
	if (s_target_mode == TargetMode::Manual ||
		s_target_mode == TargetMode::Automatic_Position)
	{
		return global_pos == s_target_pos;
	}
	else if (s_target_mode == TargetMode::Automatic_Creature)
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
	if (s_target_mode == TargetMode::Automatic_Creature)
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
	else if (s_target_mode == TargetMode::Manual||
		s_target_mode == TargetMode::Automatic_Position)
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
	bool const bright_mode = Config::brighter_target_enabled();

	if (is_wall)
	{
		if (visibility == Visibility::Visible)
		{
			return (bright_mode) ? cstr_LighterPurple : cstr_DarkWhite;
		}
		else if (visibility == Visibility::Explored)
		{
			return (bright_mode) ? cstr_DarkPurple : cstr_DarkerGrey;
		}
	}

	return (bright_mode) ? cstr_DarkerPurple : cstr_DarkestGrey;
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
