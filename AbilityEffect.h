#pragma once

#include "SpellEffect.h"

namespace Ability
{
	using EffectParams = Spell::EffectParams;
	using EffectFunc = Spell::EffectFunc;

	void steal_bean(EffectParams params);
	void eat_bean(EffectParams params);
}
