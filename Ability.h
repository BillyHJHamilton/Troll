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
		Count,
	};

	// Clearly there's conceptual overlap between Spell and Ability code here.
	// We may merge things later on, but for now keeping it separate.
	enum class TargetType : byte
	{
		Self,		// Ability that affects the user
		Melee,		// Ability that hits a creature in an adjacent tile
		Projectile,	// Ability that shoots a projectile beam at the target

		//Creature, // Beam that continues until it hits a creature
		//Tile,     // Beam that stops at the target tile
		//Sight,    // Affects a tile without firing a beam
		//Self      // Ability that only affects the user
	};

	struct Data
	{
		Damage::Type damage_type = (Damage::Type)0;

		int damage = 0;
		int accuracy = -1; // out of 100.
		int range = 0;

		int cooldown_min = 0;
		int cooldown_max = 0;

		Ability::TargetType target_type = TargetType::Self;

		// I guess for now I'll just use the spell effect code, though.
		Spell::EffectFunc effect_func = nullptr;
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

	TargetType target_type(Ability::Index index);
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
