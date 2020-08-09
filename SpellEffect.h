#pragma once

namespace Spell
{
	using EffectFunc = void(*)(int caster, int target);
}

void relashio_effect (int caster, int target);
void flipendo_effect (int caster, int target);
void tarantallegra_effect (int caster, int target);
