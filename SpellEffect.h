#pragma once

#include "Types.h"

namespace Spell
{
	using EffectFunc = void(*)(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);

	void vermillious(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void flipendo(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void tarantallegra(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void locomotor_mortis(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void rictusempra(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void fumos(Creature::Handle caster, Creature::Handle target_unused, LineCache::Itr3D const* impact_line);
	void mimblewimble(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void lacarnum_inflamare(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void furnunculus(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void stupefy(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void impedementa(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
	void bat_bogey_hex(Creature::Handle caster, Creature::Handle target, LineCache::Itr3D const* impact_line);
}
