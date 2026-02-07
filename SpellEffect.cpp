#include "SpellEffect.h"

#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "Status.h"
#include "World.h"

#include <cassert>
#include <iostream>
#include <string>

namespace Spell
{

// Put one space at the start of the messages for these, for a hanging indent.

void vermillious (Creature::Handle caster, Creature::Handle target)
{
	Draw::creature_message(target, " " + Grammar::You_are(target) + " showered in sparks!");
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
		const int dz = World::read().get_stairs_dz(target.pos(), knock_pos.xy());
		knock_pos.z += dz;

		Creature::Handle secondary_target = Creature::creature_at_pos(knock_pos);

		if (dz > 0)
		{
			Draw::creature_message(target, " " + Grammar::You_are(target) + " knocked into the stairs!");
			target.take_damage(1, caster);
		}
		else if (World::read().is_solid(knock_pos))
		{
			Draw::creature_message(target, " " + Grammar::You_are(target) + " knocked into the wall!");
			target.take_damage(1, caster);
		}
		else if (secondary_target != Creature::None)
		{
			Draw::creature_message(target, " " + Grammar::You_are(target) + " knocked into "
				+ Grammar::you(secondary_target) + "!");
			target.take_damage(1, caster);
			secondary_target.take_damage(1, caster);
		}
		else if (dz < 0)
		{
			target.move(knock_pos);
			Draw::creature_message(target, " " + Grammar::You_are(target) + " knocked down the stairs!");
			target.take_damage(4, caster);
		}
		else
		{
			target.move(knock_pos);
			Draw::creature_message(target, " " + Grammar::You_are(target) + " knocked back!");
		}
	}
}

void tarantallegra (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::Dancing))
	{
		Draw::creature_message(target, " " + Grammar::Your(target) + " feet quicken their dance!");
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
			Draw::creature_message(target, " " + Grammar::Your(target) + " legs partially loosen.");
		}
		else
		{
			Draw::creature_message(target, " " + Grammar::Your(target) + " feet dance!");
			target.inflict_status(Status::Dancing, apply_amount);
		}
	}
}

void locomotor_mortis (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::LegLocked))
	{
		Draw::creature_message(target, " " + Grammar::Your(target) + " legs are more tightly locked together!");
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
			Draw::creature_message(target, " " + Grammar::Your(target) + " feet dance more slowly.");
		}
		else
		{
			Draw::creature_message(target, " " + Grammar::Your(target) + " legs are locked together!");
			target.inflict_status(Status::LegLocked, apply_amount);
		}
	}
}

void rictusempra (Creature::Handle caster, Creature::Handle target)
{
	Draw::creature_message(target, " Something is tickling " + Grammar::you(target) + "!");
	target.inflict_status(Status::Tickled, Random::in_range(4,8));
}

void mimblewimble (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::TongueTied))
	{
		Draw::creature_message(target, " " + Grammar::You(target) + " " +
			Grammar::verbs("become", target) + " more tongue-tied.");
	}
	else
	{
		Draw::creature_message(target, " " + Grammar::You(target) + " " +
			Grammar::verbs("become", target) + " tongue-tied.");
	}
	target.inflict_status(Status::TongueTied, 5);
}

void lacarnum_inflamare (Creature::Handle caster, Creature::Handle target)
{
	if (target.has_status(Status::Burning))
	{
		Draw::creature_message(target, " " + Grammar::Your(target) + " clothes are burning in more places!");
	}
	else
	{
		Draw::creature_message(target, " " + Grammar::Your(target) + " clothes burst into flames!");
	}

	target.inflict_status(Status::Burning, 5);
}

void furnunculus (Creature::Handle caster, Creature::Handle target)
{
	Draw::creature_message(target, " " + Grammar::Your(target) + " skin boils!");
}

}
