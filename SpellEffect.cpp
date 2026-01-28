#include "SpellEffect.h"

#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Status.h"
#include "World.h"

#include <cassert>
#include <iostream>
#include <string>

namespace Spell
{

void vermillious (Creature::Handle caster, Creature::Handle target)
{
	std::string message = Grammar::You_are(target) + " showered in sparks!";
	Draw::add_message(std::move(message));
}

void flipendo (Creature::Handle caster, Creature::Handle target)
{
	// Push back
	std::optional<LineCache::Itr3D> optional_line = Beam::get_latest_impact_line();
	if (optional_line.has_value())
	{
		LineCache::Itr3D& line = *optional_line;
		line.advance_and_loop();
		Vec3 knock_pos = *line;

		// check for collision
		World const& world = World::read();
		if (world.is_solid(knock_pos))
		{
			std::string message = Grammar::You_are(target) + " knocked into the wall!";
			Draw::add_message(std::move(message));
			target.take_damage(1, caster);
		}
		else
		{
			Creature::Handle secondary_target = Creature::creature_at_pos(knock_pos);
			if (secondary_target != Creature::None)
			{
				std::string message = Grammar::You_are(target) + " knocked into "
					+ Grammar::you(secondary_target) + "!";
				Draw::add_message(std::move(message));
				target.take_damage(1, caster);
				secondary_target.take_damage(1, caster);
			}
			else
			{
				target.move(knock_pos);
				std::string message = Grammar::You_are(target) + " knocked back!";
				Draw::add_message(std::move(message));
			}
		}
	}


}

void tarantallegra (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::Dancing))
	{
		Draw::add_message(Grammar::Your(target) + " feet quicken their dance!");
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
			Draw::add_message(Grammar::Your(target) + " legs partially loosen.");
		}
		else
		{
			Draw::add_message(Grammar::Your(target) + " feet dance!");
			target.inflict_status(Status::Dancing, apply_amount);
		}
	}
}

void locomotor_mortis(Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::LegLocked))
	{
		Draw::add_message(Grammar::Your(target) + " legs are more tightly locked together!");
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
			Draw::add_message(Grammar::Your(target) + " feet dance more slowly.");
		}
		else
		{
			Draw::add_message(Grammar::Your(target) + " legs are locked together!");
			target.inflict_status(Status::LegLocked, apply_amount);
		}
	}
}

}