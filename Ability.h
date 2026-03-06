#pragma once

#include "Types.h"
#include "Geometry.h"

// Abilities are similar to spells, but with no miscasts or magic skill involved.
namespace Ability
{
	enum Index : int
	{
		None = c_Invalid,
		StealBean,
		EatBean,
		Count,
	};

	// Clearly there's conceptual overlap between Spell and Ability code here.
	// We may merge things later on, but for now keeping it separate.
	enum class TargetType : byte
	{
		Melee,		// Ability that hits a creature in an adjacent tile
		Self,		// Ability that affects the user

		//Creature, // Beam that continues until it hits a creature
		//Tile,     // Beam that stops at the target tile
		//Sight,    // Affects a tile without firing a beam
		//Self      // Ability that only affects the user
	};

	struct Data
	{
		int damage;
		int accuracy; // out of 100.
		int range;

		Ability::TargetType target_type;

		// I guess for now I'll just use the spell effect code, though.
		Spell::EffectFunc effect_func;
	};

	//-------------------------------------------------------------------------
	// Global interface

	void init();

	bool is_valid(Ability::Index index);

	int get_damage(Ability::Index index);
	bool is_damaging(Ability::Index index);

	TargetType target_type(Ability::Index index);
	int get_accuracy (Ability::Index index);
	int get_range (Ability::Index index);
	bool in_range (Ability::Index index, Vec3 origin, Vec3 target);
	Spell::EffectFunc get_effect_func (Ability::Index index);

	void execute_effect(Ability::Index index, Spell::EffectParams params);

} // namespace Ability
