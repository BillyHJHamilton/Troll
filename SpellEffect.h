#pragma once

namespace Spell
{
	using EffectFunc = void(*)(Creature::Handle caster, Creature::Handle target);
}

void vermillious_effect (Creature::Handle caster, Creature::Handle target);
void flipendo_effect (Creature::Handle caster, Creature::Handle target);
void tarantallegra_effect (Creature::Handle caster, Creature::Handle target);
