#include "SpellEffect.h"

#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Map.h"
#include "Status.h"

#include <iostream>
#include <string>

void vermillious_effect (Creature::Handle caster, Creature::Handle target)
{
	std::string message = Grammar::You_are(target) + " showered in sparks!";
	add_game_message(std::move(message));
}

void flipendo_effect (Creature::Handle caster, Creature::Handle target)
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
			target.take_damage(1, caster);
		}
		else
		{
			Creature::Handle secondary_target = Creature::creature_at_pos(knock_pos);
			if (secondary_target != Creature::None)
			{
				std::string message = Grammar::You_are(target) + " knocked into "
					+ Grammar::you(secondary_target) + "!";
				add_game_message(std::move(message));
				target.take_damage(1, caster);
				secondary_target.take_damage(1, caster);
			}
			else
			{
				target.move(knock_pos);
				std::string message = Grammar::You_are(target) + " knocked back!";
				add_game_message(std::move(message));
			}
		}
	}


}

void tarantallegra_effect (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::Dancing))
	{
		add_game_message(Grammar::Your(target) + " feet quicken their dance!");
		target.inflict_status(Status::Dancing, 4);
	}
	else
	{
		int apply_amount = 4;
		if (target.has_status(Status::LegLocked))
		{
			apply_amount -= target.status_severity(Status::LegLocked);
			target.reduce_status(Status::LegLocked, 4);
		}

		if (target.has_status(Status::LegLocked))
		{
			add_game_message(Grammar::Your(target) + " legs partially loosen.");
		}
		else
		{
			add_game_message(Grammar::Your(target) + " feet dance!");
			target.inflict_status(Status::Dancing, apply_amount);
		}
	}
}

void locomotor_mortis_effect(Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::LegLocked))
	{
		add_game_message(Grammar::Your(target) + " legs are more tightly locked together!");
		target.inflict_status(Status::LegLocked, 4);
	}
	else
	{
		int apply_amount = 4;
		if (target.has_status(Status::Dancing))
		{
			apply_amount -= target.status_severity(Status::LegLocked);
			target.reduce_status(Status::Dancing, 4);
		}

		if (target.has_status(Status::Dancing))
		{
			add_game_message(Grammar::Your(target) + " feet dance more slowly.");
		}
		else
		{
			add_game_message(Grammar::Your(target) + " legs are locked together!");
			target.inflict_status(Status::LegLocked, apply_amount);
		}
	}
}
