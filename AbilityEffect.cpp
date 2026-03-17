#include "AbilityEffect.h"

#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
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
		if (target.is_player())
		{
			int const bean_slot = Inventory::read().find_first_item(Item::BBBean);
			if (bean_slot != c_Invalid)
			{
				// do stuff
				Item::Handle item = Inventory::edit().pop_item(bean_slot);
				user.push_item(item);

				Draw::creature_message(user, std::format("{} {} one of {} beans!",
					Grammar::You(user), Grammar::verbs("grab", user), Grammar::your(target)));
			}
			else
			{
				Draw::creature_message(user, std::format("{} {} {}, searching for beans.",
					Grammar::You(user), Grammar::verbs("sniff", user), Grammar::you(target)));
			}
		}
		else
		{
			// Todo
			DebugBreak();
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
	Creature::Handle const target = params.target;
	if (Check(target.valid()))
	{
		Draw::creature_message(target, std::format("{} burned!",
			Grammar::You_are(target)));
	}
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

}
