#pragma once

#include "Types.h"

namespace Spell
{
	using EffectFunc = void(*)(Creature::Handle caster, Creature::Handle target);

	void vermillious(Creature::Handle caster, Creature::Handle target);
	void flipendo(Creature::Handle caster, Creature::Handle target);
	void tarantallegra(Creature::Handle caster, Creature::Handle target);
	void locomotor_mortis(Creature::Handle caster, Creature::Handle target);
	void rictusempra(Creature::Handle caster, Creature::Handle target);
	void mimblewimble(Creature::Handle caster, Creature::Handle target);
	void lacarnum_inflamare(Creature::Handle caster, Creature::Handle target);
	void furnunculus(Creature::Handle caster, Creature::Handle target);
	void stupefy(Creature::Handle caster, Creature::Handle target);
	void impedementa(Creature::Handle caster, Creature::Handle target);
	void bat_bogey_hex(Creature::Handle caster, Creature::Handle target);
}
