#include "SpellEffect.h"

#include "Beam.h"
#include "Cloud.h"
#include "Creature.h"
#include "Damage.h"
#include "Draw.h"
#include "Feature.h"
#include "Grammar.h"
#include "Inventory.h"
#include "Random.h"
#include "Spell.h"
#include "Status.h"
#include "Terrain.h"
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

	Draw::creature_message(target, std::format("{0} showed in sparks!",
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
		Draw::creature_message(target, std::format("{0} knocked into the stairs!",
			Grammar::You_are(target)));
		target.take_damage({1, Damage::Basic, Damage::Cause(caster)});
	}
	else if ((target.type() == Creature::FireCrab || target.type() == Creature::BigFireCrab)
		&& !target.has_status(Status::Prone) && Random::coinflip())
	{
		Draw::creature_message(target, std::format("{} flipped on its back!",
			Grammar::You_are(target)));
		target.inflict_status(Status::Prone, 1);
	}
	else if (target.type() != Creature::FireCrab && target.type() != Creature::BigFireCrab &&
		target.has_status(Status::LegLocked) && !target.has_status(Status::Prone)
		&& Random::in_range(1,10) <= target.status_severity(Status::LegLocked))
	{
		Draw::creature_message(target, std::format("{} {} down!",
			Grammar::You(target), Grammar::verbs("fall", target)));
		target.take_damage({1, Damage::Basic, Damage::Cause(caster)});
		target.inflict_status(Status::Prone, 1);
	}
	else if (World::read().is_solid(knock_pos))
	{
		Draw::creature_message(target, std::format("{0} knocked into the wall!",
			Grammar::You_are(target)));
		target.take_damage({1, Damage::Basic, Damage::Cause(caster)});
	}
	else if (secondary_target != Creature::None)
	{
		Draw::creature_message(target, std::format("{0} knocked into {1}!",
			Grammar::You_are(target), Grammar::you(secondary_target)));
		target.take_damage({1, Damage::Basic, Damage::Cause(caster)});
		secondary_target.take_damage({1, Damage::Basic, Damage::Cause(caster)});
	}
	else if (dz < 0)
	{
		Draw::creature_message(target, std::format("{0} knocked down the stairs!",
			Grammar::You_are(target)));
		target.move(knock_pos);
		target.take_damage({4, Damage::Basic, Damage::Cause(caster)});
		Draw::draw_screen();
		Draw::anim_delay();
	}
	else
	{
		Draw::creature_message(target, std::format("{0} knocked back!",
			Grammar::You_are(target)));
		target.move(knock_pos);
		Draw::draw_screen();
		Draw::anim_delay();
	}
}

void alohomora(EffectParams params)
{
	Vec3 const pos = params.target_pos;
	Terrain::Type t = World::read().get_terrain(pos);
	if (t == Terrain::Chest)
	{
		Feature::open_chest(pos);
	}
	else if (t == Terrain::Portrait)
	{
		Feature::open_portrait(pos);
	}
	else
	{
		Draw::pos_message(pos, "It has no effect.");
	}
}

void tarantallegra (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_tag(Creature::Tag::Immune_Legs))
	{
		Draw::creature_message(target, std::format("{} unaffected.", 
			Grammar::You_are(target)));
	}
	else if (target.has_status(Status::Calm))
	{
		Draw::creature_message(target, std::format("{} {} calm.",
			Grammar::You(target), Grammar::verbs("remain", target)));
	}
	else if (target.has_status(Status::Dancing))
	{
		Draw::creature_message(target, std::format("{0} feet quicken their dance!",
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
			Draw::creature_message(target, std::format("{0} legs partially loosen.",
				Grammar::Your(target)));
		}
		else
		{
			Draw::creature_message(target, std::format("{0} feet dance!",
				Grammar::Your(target)));
			target.inflict_status(Status::Dancing, apply_amount);
		}
	}
}

void locomotor_mortis (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_tag(Creature::Tag::Immune_Legs))
	{
		Draw::creature_message(target, std::format("{} unaffected.",
			Grammar::You_are(target)));
	}
	else if (target.has_status(Status::LegLocked))
	{
		Draw::creature_message(target,
			std::format("{0} legs are more tightly locked together!",
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
			Draw::creature_message(target, std::format("{0} feet dance more slowly.",
				Grammar::Your(target)));
		}
		else
		{
			Draw::creature_message(target, std::format("{0} legs are locked together!",
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
		Draw::creature_message(target, std::format("{} {} calm.",
			Grammar::You(target), Grammar::verbs("remain", target)));
	}
	else
	{
		Draw::creature_message(target, std::format("Something is tickling {0}!",
			Grammar::you(target)));
		target.inflict_status(Status::Tickled, Random::in_range(4,6));
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
		Draw::add_message("Smoke billows forth.");
	}
}

void mimblewimble (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.num_spells() == 0)
	{
		Draw::creature_message(target, std::format("{} unaffected.", 
			Grammar::You_are(target)));
	}
	else
	{
		char const* fmt = target.has_status(Status::TongueTied) ?
			"{0} {1} more tongue-tied." :
			"{0} {1} tongue-tied.";
	
		std::string s1 = Grammar::You(target);
		std::string s2 = Grammar::verbs("become", target);
		Draw::creature_message(target, std::vformat(fmt, std::make_format_args(s1,s2)));

		target.inflict_status(Status::TongueTied, 5);
	}
}

void lacarnum_inflamare (EffectParams params)
{
	Creature::Handle caster = params.caster;
	Creature::Handle target = params.target;

	if (target.has_tag(Creature::Tag::Immune_Clothes))
	{
		Draw::creature_message(target, std::format("{} burned!",
			Grammar::You_are(target)));

		Damage::Packet const dmg
		{
			.amount = Random::in_range(1,3),
			.type = Damage::Fire,
			.cause = Damage::Cause(caster)
		};
		target.take_damage(dmg);
	}
	else
	{
		char const* fmt = target.has_status(Status::Burning) ?
			"{0} clothes are burning in more places!" :
			"{0} clothes burst into flames!";
		std::string s1 = Grammar::Your(target);
		Draw::creature_message(target, std::vformat(fmt, std::make_format_args(s1)));

		target.inflict_status(Status::Burning, 5);
	}
}

void furnunculus (EffectParams params)
{
	Creature::Handle target = params.target;
	Draw::creature_message(target, std::format("{0} skin boils!",
		Grammar::Your(target)));
}

// helper function
void finite_option(std::vector<Status::Index>& list, Creature::Handle caster, Status::Index status)
{
	if (caster.has_status(status))
	{
		list.push_back(status);
	}
}

void finite_incantatem (EffectParams params)
{
	Creature::Handle caster = params.caster;
	std::vector<Status::Index> options;
	options.reserve(7);

	finite_option(options, caster, Status::Dancing);
	finite_option(options, caster, Status::LegLocked);
	finite_option(options, caster, Status::Tickled);
	finite_option(options, caster, Status::TongueTied);
	finite_option(options, caster, Status::Impeded);
	finite_option(options, caster, Status::Batty);
	//finite_option(options, caster, Status::Confused);
	//finite_option(options, caster, Status::Water);

	// while you're adding to this list, add to the list in the
	// Bot::spell_is_useless() as well

	if (!options.empty())
	{
		Status::Index to_cure = Random::from_vector(options);
		caster.cure_status(to_cure);
	}
	else
	{
		Draw::creature_message(caster, " Nothing happens.");
	}
}

void accio (EffectParams params)
{
	// TODO It would be great to show the item fly through the air.
	// And in theory, it could hit someone and stop, dealing damage based on its heft.

	// TODO The grammar for items is more complex.  Problem cases:
	//  Harry hangs onto his Harry's notes  ->  Harry hangs onto his notes
	//  Harry's notes flies into your hand    ->  Harry's notes fly into your hand
	// Maybe we need a function to get indefinite: "a potion flies..." "his potion" "a sheaf of parchment"

	Creature::Handle caster = params.caster;
	Creature::Handle target = params.target;
	Vec3 const pos = params.target_pos;

	Item::Handle summon_item;

	if (target.valid())
	{
		if (target.has_item())
		{
			if (Random::one_in(3))
			{
				Draw::creature_message(target, std::format(
					"{} {} onto {} belongings.",
					Grammar::You(target), Grammar::verbs("hang", target),
					Grammar::your_pr(target)));
				return;
			}
			else
			{
				summon_item = target.pop_item();
			}
		}

		// TODO Perhaps at higher spell levels, it could actually pull a creature towards you?
	}

	if (!summon_item.valid())
	{
		summon_item = World::edit().pop_item(pos);
	}

	if (summon_item.valid())
	{
		if (caster.is_player())
		{
			Inventory::edit().add_item(summon_item);
		}
		else
		{
			caster.push_item(summon_item);
		}

		Draw::creature_message(caster, std::format("Whoosh!  {} got {}!",
			Grammar::You(caster), summon_item.name()));
		return;
	}
	else if (target.valid())
	{
		Draw::creature_message(caster, "Nothing happened.");
	}
}

void stupefy (EffectParams params)
{
	Creature::Handle const caster = params.caster;
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

	Draw::creature_message(target, std::format("{0} struck by a {1}!",
		Grammar::You_are(target), bolt_description));
}

void impedementa (EffectParams params)
{
	Creature::Handle target = params.target;

	if (target.has_status(Status::Impeded))
	{
		Draw::creature_message(target, std::format("{0} movement is further impeded!",
			Grammar::Your(target)));
	}
	else
	{
		Draw::creature_message(target, std::format("A force impedes {0} movement!",
			Grammar::your(target)));
	}
	target.inflict_status(Status::Impeded, 5);
}

void bat_bogey_hex (EffectParams params)
{
	Creature::Handle target = params.target;

	char const* fmt = target.has_status(Status::Batty) ?
		"The swarm of black winged things around {0} thickens!" :
		"A swarm of black winged things descends on {0}!";
	std::string s1 = Grammar::you(target);
	Draw::creature_message(target, std::vformat(fmt, std::make_format_args(s1)));

	target.inflict_status(Status::Batty, 6);
}

}
