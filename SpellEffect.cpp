#include "SpellEffect.h"

#include "Beam.h"
#include "Cloud.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "World.h"

#include <cassert>
#include <iostream>
#include <format>

namespace Spell
{

// Put one space at the start of the messages for these, for a hanging indent.
// Be aware that impact_line MAY be nullptr, particularly if spell is self-targeted.
// Target may be Creature::None, if spell hit self or detonated in midair.

void vermillious (EffectParams params)
{
	Creature::Handle const target = params.target;
	Draw::creature_message(target, std::format(" {0} showed in sparks!",
		Grammar::You_are(target)));
}

void flipendo (EffectParams params)
{
	Creature::Handle const caster = params.caster;
	Creature::Handle target = params.target;

	Vec3 knock_pos;

	if (params.impact_line == nullptr)
	{
		// If you shoot yourself, just push in a random direction.
		CompassDirection dir = Random::compass_direction(false);
		knock_pos = params.target_pos + c_Compass[dir].xy0();
	}
	else
	{
		LineCache::Itr3D line = *params.impact_line; // copy
		line.advance_and_loop();
		knock_pos = *line;
	}

	// Push back
	const int dz = World::read().get_stairs_dz(target.pos(), knock_pos.xy());
	knock_pos.z += dz;

	Creature::Handle secondary_target = Creature::creature_at_pos(knock_pos);

	if (dz > 0)
	{
		Draw::creature_message(target, std::format(" {0} knocked into the stairs!",
			Grammar::You_are(target)));
		target.take_damage(1, caster);
	}
	else if (World::read().is_solid(knock_pos))
	{
		Draw::creature_message(target, std::format(" {0} knocked into the wall!",
			Grammar::You_are(target)));
		target.take_damage(1, caster);
	}
	else if (secondary_target != Creature::None)
	{
		Draw::creature_message(target, std::format(" {0} knocked into {1}!",
			Grammar::You_are(target), Grammar::you(secondary_target)));
		target.take_damage(1, caster);
		secondary_target.take_damage(1, caster);
	}
	else if (dz < 0)
	{
		Draw::creature_message(target, std::format(" {0} knocked down the stairs!",
			Grammar::You_are(target)));
		target.move(knock_pos);
		target.take_damage(4, caster);
	}
	else
	{
		Draw::creature_message(target, std::format(" {0} knocked back!",
			Grammar::You_are(target)));
		target.move(knock_pos);
	}
}

void tarantallegra (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_status(Status::Calm))
	{
		Draw::creature_message(target, std::format(" {} {} calm.",
			Grammar::You(target), Grammar::verbs("remain", target)));
	}
	else if (target.has_status(Status::Dancing))
	{
		Draw::creature_message(target, std::format(" {0} feet quicken their dance!",
			Grammar::Your(target)));
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
			Draw::creature_message(target, std::format(" {0} legs partially loosen.",
				Grammar::Your(target)));
		}
		else
		{
			Draw::creature_message(target, std::format(" {0} feet dance!",
				Grammar::Your(target)));
			target.inflict_status(Status::Dancing, apply_amount);
		}
	}
}

void locomotor_mortis (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_status(Status::LegLocked))
	{
		Draw::creature_message(target,
			std::format(" {0} legs are more tightly locked together!",
			Grammar::Your(target)));
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
			Draw::creature_message(target, std::format(" {0} feet dance more slowly.",
				Grammar::Your(target)));
		}
		else
		{
			Draw::creature_message(target, std::format(" {0} legs are locked together!",
				Grammar::Your(target)));
			target.inflict_status(Status::LegLocked, apply_amount);
		}
	}
}

void rictusempra (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_status(Status::Calm))
	{
		Draw::creature_message(target, std::format(" {} {} calm.",
			Grammar::You(target), Grammar::verbs("remain", target)));
	}
	else
	{
		Draw::creature_message(target, std::format(" Something is tickling {0}!",
			Grammar::you(target)));
		target.inflict_status(Status::Tickled, Random::in_range(4,8));
	}
}

void fumos (EffectParams params)
{
	Vec3 const target_pos = params.target_pos;

	bool msg = false;
	for (CompassItr itr(true); itr; ++itr)
	{
		CompassDirection dir = *itr;
		Vec3 const cloud_pos = target_pos + c_Compass[dir].xy0();
			
		if (World::read().is_solid(cloud_pos))
		{
			continue;
		}

		if (dir == c_CompassNoMove || Random::coinflip())
		{
			int const lifetime = Random::in_range(8,11);
			bool const added = World::edit().try_add_cloud(cloud_pos, Cloud::Smoke, lifetime);
			if (!msg && added && World::read().is_visible(cloud_pos))
			{
				msg = true;
			}
		}
	}

	if (msg)
	{
		Draw::add_message(" Smoke billows forth.");
	}
}

void mimblewimble (EffectParams params)
{
	Creature::Handle target = params.target;

	char const* fmt = target.has_status(Status::TongueTied) ?
		" {0} {1} more tongue-tied." :
		" {0} {1} tongue-tied.";
	Draw::creature_message(target, std::vformat(fmt, std::make_format_args(
		Grammar::You(target), Grammar::verbs("become", target))));

	target.inflict_status(Status::TongueTied, 5);
}

void lacarnum_inflamare (EffectParams params)
{
	Creature::Handle target = params.target;

	char const* fmt = target.has_status(Status::Burning) ?
		" {0} clothes are burning in more places!" :
		" {0} clothes burst into flames!";
	Draw::creature_message(target, std::vformat(fmt, std::make_format_args(
		Grammar::Your(target))));

	target.inflict_status(Status::Burning, 5);
}

void furnunculus (EffectParams params)
{
	Creature::Handle target = params.target;
	Draw::creature_message(target, std::format(" {0} skin boils!",
		Grammar::Your(target)));
}

void stupefy (EffectParams params)
{
	Creature::Handle const caster = params.target;
	Creature::Handle const target = params.target;

	char const* bolt_description;
	if (Spell::get_damage(Spell::Stupefy, caster) > 10)
		bolt_description = "spectacular bolt of red light";
	else if (Spell::get_damage(Spell::Stupefy, caster) > 8)
		bolt_description = "mighty bolt of red light";
	else if (Spell::get_damage(Spell::Stupefy, caster) > 6)
		bolt_description = "strong bolt of red light";
	else if (Spell::get_damage(Spell::Stupefy, caster) > 4)
		bolt_description = "solid bolt of red light";
	else if (Spell::get_damage(Spell::Stupefy, caster) > 3)
		bolt_description = "bolt of red light";
	else
		bolt_description = "weak bolt of red light";

	Draw::creature_message(target, std::format(" {0} struck by a {1}!",
		Grammar::You_are(target), bolt_description));
}

void impedementa (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_status(Status::Impeded))
	{
		Draw::creature_message(target, std::format(" {0} movement is further impeded!",
			Grammar::Your(target)));
	}
	else
	{
		Draw::creature_message(target, std::format(" A force impedes {0} movement!",
			Grammar::your(target)));
	}
	target.inflict_status(Status::Impeded, 5);
}

void bat_bogey_hex (EffectParams params)
{
	Creature::Handle target = params.target;

	char const* fmt = target.has_status(Status::Batty) ?
		" The swarm of black winged things around {0} thickens!" :
		" A swarm of black winged things descends on {0}!";
	Draw::creature_message(target, std::vformat(fmt, std::make_format_args(
		Grammar::you(target))));

	target.inflict_status(Status::Batty, 6);
}

}
