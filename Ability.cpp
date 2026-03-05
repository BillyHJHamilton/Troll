#include "Ability.h"

#include "AbilityEffect.h"
#include "Debug.h"

#include <array>

namespace Ability
{

static std::array<Ability::Data,Ability::Count> constexpr s_ability_list =
{	//					Dmg	Acc	Rng	TargetType			EffectFunc
	/* StealBean */	  {	0,	50,	1,	TargetType::Melee,	&steal_bean }
};

void init()
{
	// none required for now
}

bool is_valid(Ability::Index index)
{
	return index > Ability::None && index < Ability::Count;
}

int get_damage(Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).damage;
}

bool is_damaging(Ability::Index index)
{
	return get_damage(index) > 0;
}

TargetType target_type(Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).target_type;
}

int get_accuracy (Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).accuracy;
}

int get_range (Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).range;
}

bool in_range (Ability::Index index, Vec3 origin, Vec3 target)
{
	if (target_type(index) == TargetType::Melee)
	{
		return chessboard_adjacent(origin.xy(), target.xy());
	}

	DebugBreak();
	return false;
}

Spell::EffectFunc get_effect_func (Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).effect_func;
}

} // namespace Ability
