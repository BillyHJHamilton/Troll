#include "AbilityEffect.h"

#include "Ability.h"
#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Feature.h"
#include "Inventory.h"
#include "Random.h"
#include "Status.h"

#include <format>

namespace Ability
{

void steal_bean(EffectParams params)
{
	Creature::Handle user = params.caster;
	Creature::Handle target = params.target;

	if (user.valid() && target.valid())
	{
		Item::Handle bean = c_Invalid;
		if (target.is_player())
		{
			int const bean_slot = Inventory::read().find_first_item(Item::BBBean);
			if (bean_slot != c_Invalid)
			{
				bean = Inventory::edit().pop_item(bean_slot);
			}
		}
		else
		{
			if (target.peek_item().type() == Item::BBBean)
			{
				bean = target.pop_item();
			}
		}

		if (bean.valid())
		{
			user.push_item(bean);

			Draw::creature_message(user, std::format("{} {} one of {} beans!",
				Grammar::You(user), Grammar::verbs("grab", user), Grammar::your(target)));
		}
		else
		{
			if (Random::coinflip())
			{
				Draw::creature_message(user, std::format("{} {} {}, searching for beans.",
					Grammar::You(user), Grammar::verbs("sniff", user), Grammar::you(target)));
			}
			else
			{
				target.take_damage({1,Damage::Basic,Damage::Cause(user)});
				Draw::creature_message(user, std::format("{} {} {}.",
					Grammar::You(user), Grammar::verbs("hit", user), Grammar::you(target)));
			}
		}
	}
}

void eat_bean(EffectParams params)
{
	Creature::Handle user = params.caster;
	if (Check(user.valid()))
	{
		if (user.peek_item().type() == Item::BBBean)
		{
			Draw::creature_message(user, std::format("{} {} a bean!",
				Grammar::You(user), Grammar::verbs("eat", user)));

			Item::Handle item = user.pop_item();
			item.destroy();
		}
		else
		{
			Draw::creature_message(user, std::format("{} {} hungry.",
				Grammar::You(user), Grammar::feel(user)));
		}
	}
}

void headbutt(EffectParams params)
{
	Creature::Handle const user = params.caster;
	Creature::Handle const target = params.target;
	if (user.valid() && target.valid())
	{
		Draw::creature_message(target, std::format("{} {} {}!",
			Grammar::You(user), Grammar::verbs("headbutt",user), Grammar::you(target)));
	}
}

void fire_gob_hit(EffectParams params)
{
	if (params.target.valid())
	{
		// fire gob vs creature

		Creature::Handle const target = params.target;
		if (Check(target.valid()))
		{
			Draw::creature_message(target, std::format("{} burned!",
				Grammar::You_are(target)));
		}
	}

	// fire gob vs feature
	Feature::hit_by_fire(params.target_pos, Ability::get_damage(Ability::ShootFire));
}

void doxy_bite(EffectParams params)
{
	Creature::Handle const user = params.caster;
	Creature::Handle target = params.target;
	if (user.valid() && target.valid())
	{
		Draw::creature_message(target, std::format("{} {} {} a nasty bite!",
			Grammar::You(user), Grammar::verbs("give",user), Grammar::you(target)));

		target.inflict_status(Status::Venom, Random::in_range(3,4));
	}
}

void trip_kick(EffectParams params)
{
	Creature::Handle const user = params.caster;
	Creature::Handle target = params.target;
	if (user.valid() && target.valid())
	{
		if (target.has_tag(Creature::Tag::Immune_Legs) ||
			target.has_status(Status::Prone))
		{
			Draw::creature_message(target, std::format("{} {} {}!",
				Grammar::You(user), Grammar::verbs("kick",user), Grammar::you(target)));
		}
		else
		{
			Draw::creature_message(target, std::format("{} {} {}!",
				Grammar::You(user), Grammar::verbs("trip",user), Grammar::you(target)));
			target.inflict_status(Status::Prone, 1);
		}
	}
}

void scratch(EffectParams params)
{
	Creature::Handle const user = params.caster;
	Creature::Handle const target = params.target;
	if (user.valid() && target.valid())
	{
		Draw::creature_message(target, std::format("{} {} {}!",
			Grammar::You(user), Grammar::verbs("scratche",user), Grammar::you(target)));
	}
}

void believe(EffectParams params)
{
	Creature::Handle user = params.caster;
	if (user.valid())
	{
		user.heal_hp(Random::in_range(5,10));

		// 50% chance to cure each harmful status
		for (int i = 0; i < Status::Count; ++i)
		{
			Status::Index status = (Status::Index)i;
			if (Status::is_harmful(status) &&
				user.has_status(status) &&
				Random::coinflip())
			{
				user.cure_status(status);
			}
		}

		Draw::creature_message(user, std::format("{} {} in {}!",
			Grammar::You(user), Grammar::verbs("believe", user),
			Grammar::format_name(user, {.mode=Grammar::NameParam::ReflexivePronoun})));
	}
}

char const* random_karate_move()
{
	switch(Random::in_range(0,39))
	{
		case 0: return "a karate chop";
		case 1: return "a spinning kick";
		case 2: return "a middle-finger knuckle fist";
		case 3: return "a palm strike";
		case 4: return "a rising mountain punch";
		case 5: return "a flowering scissor punch";
		case 6: return "a stomping joint kick";
		case 7: return "a heel-drop knee kick";
		case 8: return "a double elbow energy strike";
		case 9: return "a sudden jump kick";
		case 10: return "a forbidden dragon punch";
		case 11: return "an inverted spiral kick";
		case 12: return "a twisted wrist strike";
		case 13: return "an elegant water-spider kick";
		case 14: return "a shocking force punch";
		case 15: return "a backhand thrust";
		case 16: return "a knife-blade finger swipe";
		case 17: return "a double-finger knockout punch";
		case 18: return "a bent wrist double strike";
		case 19: return "a knee kick";
		case 20: return "a triple reflex punch";
		case 21: return "a power stance reversal";
		case 22: return "a hidden lightning kick";
		case 23: return "a revolving door kick";
		case 24: return "an underwater mountain punch";
		case 25: return "a flowing ocean chop";
		case 26: return "a gentle wind-strike fist";
		case 27: return "a spinning shin strike";
		case 28: return "a knee-drop heel kick";
		case 29: return "a rolling elbow smash";
		case 30: return "a tiger claw punch";
		case 31: return "a slack-jaw chin strike";
		case 32: return "a full-body hip smack";
		case 33: return "a focused energy stomach slam";
		case 34: return "a whip-type shoelace kick";
		case 35: return "a high-tension sleeve strike";
		case 36: return "a rippling muscle punch";
		case 37: return "a blossoming lotus kick";
		case 38: return "a flying punch";
		default: return "a secret karate move";
	}
}

void karate(EffectParams params)
{
	Creature::Handle const user = params.caster;
	Creature::Handle const target = params.target;
	if (user.valid() && target.valid())
	{
		Draw::creature_message(target, std::format("{} {} {} with {}!",
			Grammar::You(user), Grammar::verbs("hit", user), Grammar::you(target),
			random_karate_move()));
	}
}


}
