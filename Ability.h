#pragma once

#include "Types.h"

#include "Creature.h"
#include "Damage.h"
#include "Geometry.h"

// Abilities are similar to spells, but with no miscasts or magic skill involved.
namespace Ability
{
	enum Index : int
	{
		None = c_Invalid,
		StealBean,	// Gnome
		EatBean,	// Gnome
		Headbutt,	// Streeler
		ShootFire,	// Fire crab
		DoxyBite,	// Doxy
		TripKick,	// Imp
		Scratch,	// Imp
		Believe,	// MarySue
		Karate,		// MarySue
		Count,
	};

	struct Data
	{
		Damage::Type damage_type = (Damage::Type)0;

		int damage = 0;
		int accuracy = -1; // out of 100.
		int range = 0;

		int cooldown_min = 0;
		int cooldown_max = 0;

		// I guess for now I'll just use the spell effect code, though.
		Spell::EffectFunc effect_func = nullptr;

		Target::Type target_type = (Target::Type)0;
		uint target_flags = 0;
	};

	struct ProjectileData
	{
		char const* shoot_verb = nullptr;
		char const* noun = nullptr;
		char const* colour = nullptr;
		int codepoint = '*';
	};

	//-------------------------------------------------------------------------
	// Global interface

	void init();
	void clear();
	void serialize(ISerializer& s);

	bool is_valid(Ability::Index index);

	int get_damage(Ability::Index index);
	bool is_damaging(Ability::Index index);
	Damage::Type damage_type(Ability::Index index);

	Target::Type target_type(Ability::Index index);
	uint get_target_flags(Ability::Index index);
	bool has_accuracy (Ability::Index index);
	int get_accuracy (Ability::Index index);
	int get_range (Ability::Index index);
	bool in_range (Ability::Index index, Vec3 origin, Vec3 target);
	Spell::EffectFunc get_effect_func (Ability::Index index);

	// Only valid for abilities with target type Projectile
	ProjectileData get_projectile(Ability::Index index);

	void execute_effect(Ability::Index index, Spell::EffectParams params);

	//-------------------------------------------------------------------------
	// Cooldown interface

	bool is_in_cooldown(Creature::Handle creature, Ability::Index ability);
	void start_cooldown(Creature::Handle creature, Ability::Index ability);
	void clear_cooldowns(Creature::Handle creature);
	void tick_cooldowns();

} // namespace Ability
