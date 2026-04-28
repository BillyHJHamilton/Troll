#pragma once

#include "Types.h"
#include "Creature.h"

namespace Spell
{
	struct EffectParams
	{
		// Caster of the spell.
		Creature::Handle caster = Creature::None;

		// Creature hit by the spell.
		// May be None if the spell is self-targeting, or detonated in midair.
		Creature::Handle target = Creature::None;

		// Place where the spell detonated.  Often same as target or caster position.
		Vec3 target_pos;

		// Pointer to the spell's travel line at the moment of impact.
		// Used for Flipendo pushback, for example.
		// May be nullptr, if the spell is self-targeting.
		LineCache::Itr3D const* impact_line = nullptr;
	};

	using EffectFunc = void(*)(EffectParams params);

	void vermillious(EffectParams params);
	void flipendo(EffectParams params);
	void alohomora(EffectParams params);
	void colloportus(EffectParams params);
	void tarantallegra(EffectParams params);
	void locomotor_mortis(EffectParams params);
	void rictusempra(EffectParams params);
	void fumos(EffectParams params);
	void mimblewimble(EffectParams params);
	void lacarnum_inflamare(EffectParams params);
	void furnunculus(EffectParams params);
	void finite_incantatem(EffectParams params);
	void accio(EffectParams params);
	void stupefy(EffectParams params);
	void impedementa(EffectParams params);
	void bat_bogey_hex(EffectParams params);
}
