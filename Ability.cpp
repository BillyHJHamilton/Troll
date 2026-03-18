#include "Ability.h"

#include "AbilityEffect.h"
#include "Colour.h"
#include "Damage.h"
#include "Debug.h"
#include "MapUtil.h"
#include "Random.h"
#include "Serialize.h"

#include <array>

namespace Ability
{

struct CooldownEntry
{
	Creature::Handle creature = c_Invalid;
	Ability::Index ability = Ability::None;
	int timer = 0;
};
std::vector<CooldownEntry> s_cooldowns;

static std::array<Ability::Data,Ability::Count> constexpr s_ability_list =
{	//								Damage Type		Dmg	Acc	Rng	Cooldn	TargetType				EffectFunc
	/* StealBean */	Ability::Data{	Damage::None,	0,	80,	1,	0,0,	TargetType::Melee,		&steal_bean },
	/* EatBean */	Ability::Data{	Damage::None,	0,	-1,	0,	0,0,	TargetType::Self,		&eat_bean },
	/* Headbutt */	Ability::Data{	Damage::Basic,	2,	70,	1,	0,0,	TargetType::Melee,		&headbutt },
	/* ShootFire*/	Ability::Data{	Damage::Fire,	3,	70,	6,	0,2,	TargetType::Projectile,	&fire_gob_hit},
	/* DoxyBite */	Ability::Data{	Damage::Basic,	1,	75,	1,	1,1,	TargetType::Melee,		&doxy_bite },
	/* TripKick */	Ability::Data{	Damage::Basic,	1,	70,	1,	1,2,	TargetType::Melee,		&trip_kick },
	/* Scratch */	Ability::Data{	Damage::Basic,	3,	65,	1,	0,0,	TargetType::Melee,		&scratch },
};

static std::unordered_map<Ability::Index,Ability::ProjectileData> s_projectiles;

void init()
{
	s_cooldowns.reserve(50); // Probably plenty.

	// Init projectiles.
	s_projectiles[Ability::ShootFire] = {"shoot", "gob of fire", cstr_Flame, '*'};
}

void clear()
{
	s_cooldowns.clear();
}

void serialize(ISerializer& s)
{
	s.srz_vector(s_cooldowns, "Ability Cooldowns");
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

Damage::Type damage_type(Ability::Index index)
{
	return s_ability_list.at(index).damage_type;
}

TargetType target_type(Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).target_type;
}

bool has_accuracy (Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).accuracy > 0
		&& s_ability_list.at(index).accuracy < 100;
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
	assert(is_valid(index));

	switch (target_type(index))
	{
		case TargetType::Self:
			return true;

		case TargetType::Melee:
			return chessboard_adjacent(origin.xy(), target.xy());

		case TargetType::Projectile:
			return range_2d(origin, target, get_range(index));

		default:
			DebugBreak();
			return false;
	}
}

Spell::EffectFunc get_effect_func (Ability::Index index)
{
	assert(is_valid(index));
	return s_ability_list.at(index).effect_func;
}

ProjectileData get_projectile(Ability::Index index)
{
	assert(is_valid(index) && target_type(index) == TargetType::Projectile);
	return s_projectiles.at(index);
}

void execute_effect(Ability::Index index, Spell::EffectParams params)
{
	Spell::EffectFunc func = s_ability_list[index].effect_func;
	if (func != nullptr)
	{
		func(params);
	}
}

//-------------------------------------------------------------------------
// Cooldown interface

bool is_in_cooldown(Creature::Handle creature, Ability::Index ability)
{
	for (CooldownEntry cooldown : s_cooldowns)
	{
		if (cooldown.creature == creature && cooldown.ability == ability)
		{
			return true;
		}
	}
	return false;
}

void start_cooldown(Creature::Handle creature, Ability::Index ability)
{
	if (s_ability_list[ability].cooldown_max > 0)
	{
		int const turns = Random::in_range(
			s_ability_list[ability].cooldown_min,
			s_ability_list[ability].cooldown_max);
		if (turns > 0)
		{
			s_cooldowns.push_back({creature, ability, turns});
		}
	}
}

void clear_cooldowns(Creature::Handle creature)
{
	s_cooldowns.erase(std::remove_if(s_cooldowns.begin(), s_cooldowns.end(),
			[creature](const CooldownEntry& cooldown)
			{
				return cooldown.creature == creature;
			}
		), s_cooldowns.cend());
}

void tick_cooldowns()
{
	s_cooldowns.erase(std::remove_if(s_cooldowns.begin(), s_cooldowns.end(),
			[](CooldownEntry& cooldown)
			{
				// Decrement all of them.
				--cooldown.timer;
				
				// And remove if it's past 0.
				return cooldown.timer < 0;
			}
		), s_cooldowns.cend());
}

} // namespace Ability
