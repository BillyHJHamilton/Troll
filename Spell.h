#pragma once

#include "Types.h"
#include "Scratch.h"

#include <bitset>
#include <string>
#include <vector>

// Enables an overpowered spell for debug purposes.
#if _DEBUG
	#define d_EnableMegadrill true
#else
	#define d_EnableMegadrill false
#endif

namespace Spell
{
	enum Index : int
	{
		None = -1,
		Vermillious = 0,
#if d_EnableMegadrill
		Megadrill, // Test spell
#endif
		Flipendo,
		Alohomora,
		Tarantallegra,
		LocomotorMortis,
		Rictusempra,
		Fumos,
		Mimblewimble,
		LacarnumInflamare,
		Furnunculus,
		FiniteIncantatem,
		Accio,
		Stupefy,
		Impedementa,
		// Protego,
		BatBogey,
		// Avis,
		// Oppugno,
		// Episkey,
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
		// AvadaKedavra,

		Count
	};

	using TempList = std::vector<Spell::Index,Scratch<Spell::Index>>;

	enum class TargetType : byte
	{
		Creature, // Beam that continues until it hits a creature
		Tile,     // Beam that stops at the target tile
		Sight,    // Affects a tile without needing a beam
		Self      // Spell that only affects the caster
	};

	using Bitset = std::bitset<Spell::Index::Count>;

	struct Data
	{
		char const * name;
		char const * abbrev;
		char const * colour;
		
		int difficulty;
		int dark;

		Damage::Type damage_type;

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
		std::string colour = "";
		int codepoint = 0;
		int damage = 0;
		int power = 1; // = spell level * caster skill
		int accuracy = 100;
		EffectFunc effect_func = nullptr;
	};

	// functions
	void init();

	void srz_bitset(ISerializer& s, Spell::Bitset& bitset);

	bool is_valid_index(Spell::Index index);
	std::string get_name (Spell::Index spell_index);
	std::string get_abbrev (Spell::Index spell_index);
	char const* get_colour (Spell::Index spell_index);
	int get_difficulty (Spell::Index spell_index);
	int get_dark (Spell::Index spell_index);
	int get_damage (Spell::Index spell_index, Creature::Handle caster);
	bool is_damaging (Spell::Index spell_index);
	int get_power (Spell::Index spell_index, Creature::Handle caster);
	TargetType get_target_type (Spell::Index spell_index);
	int get_accuracy (Spell::Index spell_index);
	int get_range (Spell::Index spell_index);
	EffectFunc get_effect_func (Spell::Index spell_index);
	Miscast::Category get_miscast_category (Spell::Index spell_index);
	Spell::Index get_index_by_name (std::string const & spell_name);
	Spell::Index get_index_by_abbrev (std::string const & spell_abbrev);
	char const * get_description (Spell::Index spell_index);

	float get_miscast_rate (Spell::Index spell, int skill_magic);

	//void create_and_bind_instance (Spell::Index spell, Creature::Handle caster);
	//Spell::Instance & get_current_instance ();

	TempList bitset_to_temp_list(Spell::Bitset const& bitset);

	void execute_effect(Spell::Index spell_index, Spell::EffectParams params);
}
