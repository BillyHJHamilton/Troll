#pragma once

namespace Spell
{
	using EffectFunc = void(*)(Creature::Handle caster, Creature::Handle target);

	void vermillious(Creature::Handle caster, Creature::Handle target);
	void flipendo(Creature::Handle caster, Creature::Handle target);
	void tarantallegra(Creature::Handle caster, Creature::Handle target);
	void locomotor_mortis(Creature::Handle caster, Creature::Handle target);
}
