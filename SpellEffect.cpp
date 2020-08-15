#include "SpellEffect.h"

#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Map.h"
#include "Status.h"

#include <iostream>
#include <string>

void vermillious_effect (int caster, int target)
{
	std::string message = Grammar::You_are(target) + " showered in sparks!";
	add_game_message(std::move(message));
}

void flipendo_effect (int caster, int target)
{
	// Push back
	std::optional<LineItr> optional_line = Beam::get_latest_impact_line();
	if (optional_line.has_value())
	{
		LineItr line = *optional_line;
		++ line;
		Vec2 knock_pos = *line;

		// check for collision
		if (g_map().tile_is_solid(knock_pos))
		{
			std::string message = Grammar::You_are(target) + " knocked into the wall!";
			add_game_message(std::move(message));
			damage_creature(target, 1);
		}
		else
		{
			int secondary_target = creature_at_pos(knock_pos);
			if (secondary_target != Creature::None)
			{
				std::string message = Grammar::You_are(target) + " knocked into "
					+ Grammar::you(secondary_target) + "!";
				add_game_message(std::move(message));
				damage_creature(target, 1);
				damage_creature(secondary_target, 1);
			}
			else
			{
				move_creature(target, knock_pos);
				std::string message = Grammar::You_are(target) + " knocked back!";
				add_game_message(std::move(message));
			}
		}
	}


}

void tarantallegra_effect (int caster, int target)
{
	if (creature_has_status(target, Status::Dancing))
	{
		add_game_message(Grammar::Your(target) + " feet quicken their dance!");
		inflict_status(target, Status::Dancing, 4);
	}
//	else if (creature_has_status(target, Status::LegLocked))
//	{
//		reduce_status(target, Status::LEG_LOCKED, 4);
//		if (creature_has_status(target, Status::LegLocked))
//		{
//			add_game_message(format_Name_possessive(target) + " legs partially loosen.");		
//		}
//	}
	else
	{
		add_game_message(Grammar::Your(target) + " feet dance!");
		inflict_status(target, Status::Dancing, 4);
	}
}
