#pragma once

#include "Types.h"

#include <bitset>
#include <string>
#include <vector>

namespace Spell
{
	enum Index : int
	{
		None = -1,
		Vermillious = 0,
		Flipendo,
		Tarantallegra,
		LocomotorMortis,
		Rictusempra,
		Fumos,
		Mimblewimble,
		LacarnumInflamare,
		Furnunculus,
		// FiniteIncantatem,
		Stupefy,
		Impedementa,
		// Protego,
		BatBogey,
		// Avis,
		// Opugno,
		// Episky,
		// Confundo,
		// Disillusionment,
		// Apparition,
		// SalvioHexia,
		// Extrasensory,
		// Obliviate,
		// WandArrows,
		// Disintegration,
		// WaterTrap,
		// Crucio,
		// Sectumsempra,
		// VadaKedavra,

		Count
	};

	enum class TargetType
	{
		Creature,
		Tile,
		Self
	};

	using Bitset = std::bitset<Spell::Index::Count>;

	struct Data
	{
		char const * name;
		char const * abbrev;
		char const * colour;
		
		int difficulty;
		int dark;

		int damage;
		int accuracy; // out of 100.
		int range;

		EffectFunc effect_func;
		Spell::TargetType target_type;
		Miscast::Category miscast_category;
	};

	// A spell that has been cast.
	// Once it's flying in the air, it doesn't matter who cast it or what type it is.
	// This means we can also support randomly generated spells for miscasts.
	struct Instance
	{
		std::string colour;
		int codepoint;
		int damage;
		int power; // = spell level * caster skill
		int accuracy;
		EffectFunc effect_func;
	};

	// functions
	void init();

	bool is_valid_index(Spell::Index index);
	std::string get_name (Spell::Index spell_index);
	std::string get_abbrev (Spell::Index spell_index);
	std::string get_colour (Spell::Index spell_index);
	int get_difficulty (Spell::Index spell_index);
	int get_dark (Spell::Index spell_index);
	int get_damage (Spell::Index spell_index, Creature::Handle caster);
	TargetType get_target_type (Spell::Index spell_index);
	int get_accuracy (Spell::Index spell_index);
	int get_range (Spell::Index spell_index);
	EffectFunc get_effect_func (Spell::Index spell_index);
	Miscast::Category get_miscast_category (Spell::Index spell_index);
	Spell::Index get_index_by_name (std::string const & spell_name);
	Spell::Index get_index_by_abbrev (std::string const & spell_abbrev);
	char const * get_description (Spell::Index spell_index);

	float get_miscast_rate (Spell::Index spell, int skill_magic);

	void create_and_bind_instance (Spell::Index spell, Creature::Handle caster);
	Spell::Instance & get_current_instance ();

	void execute_effect(Spell::Index spell_index, Spell::EffectParams params);
}
