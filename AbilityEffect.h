#pragma once

#include "SpellEffect.h"

namespace Ability
{
	using EffectParams = Spell::EffectParams;
	using EffectFunc = Spell::EffectFunc;

	void steal_bean(EffectParams params);
	void eat_bean(EffectParams params);
	void headbutt(EffectParams params);
	void fire_gob_hit(EffectParams params);
	void doxy_bite(EffectParams params);
	void trip_kick(EffectParams params);
	void scratch(EffectParams params);
	void believe(EffectParams params);
	void karate(EffectParams params);
}
