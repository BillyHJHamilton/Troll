#include "Spell.h"

#include "Creature.h"
#include "MiscastCategory.h"
#include "SpellEffect.h"

#include <array>

namespace Spell
{

static std::array<Spell::Data, Spell::Count> constexpr s_spell_list = 
{
	Spell::Data {"Relashio",			"RL",	"red",			5,	0,	2,		65,	&relashio_effect,			Miscast::Beam },
	Spell::Data {"Flipendo",			"FP",	"orange",		10,	0,	3,		55,	&flipendo_effect,			Miscast::Beam },
	Spell::Data {"Tarantallegra",		"TA",	"light pink",	15,	0,	0,		70,	&tarantallegra_effect,		Miscast::Beam }
};

void init()
{
	// none required, thank you constexpr
}

std::string get_name(Spell::Index spell_index)
{
	return s_spell_list[spell_index].name;
}

std::string get_abbrev(Spell::Index spell_index)
{
	return s_spell_list[spell_index].abbrev;
}

std::string get_colour(Spell::Index spell_index)
{
	return s_spell_list[spell_index].colour;
}

int get_difficulty(Spell::Index spell_index)
{
	return s_spell_list[spell_index].difficulty;
}

int get_damage(Spell::Index spell_index, int caster)
{
	int skill_magic = creature_skill_magic(caster);

	if (s_spell_list[spell_index].damage >= 0)
		return s_spell_list[spell_index].damage;
// todo stupefy damage exception
//	else if (spell_list[spell_index].damage == D_SP)
//		return 2 + (skill_magic/20); // (integer division)*/
	else
		return 0;
}

int get_accuracy(Spell::Index spell_index)
{
	return s_spell_list[spell_index].accuracy;
}

EffectFunc get_effect_func(Spell::Index spell_index)
{
	return s_spell_list[spell_index].effect_func;
}

int Spell::get_dark(Spell::Index spell_index)
{
	return s_spell_list[spell_index].dark;
}

Miscast::Category get_miscast_category(Spell::Index spell_index)
{
	return s_spell_list[spell_index].miscast_category;
}

// returns none if no spell exists with given name
Spell::Index get_index_by_name(std::string const & spell_name)
{
	// just a linear search, with multiple points of exit.
	unsigned int i = 0;
	while (i < Spell::Count)
	{
		if (s_spell_list[i].name == spell_name)
			return (Spell::Index)i;
		i++;
	}
	return Spell::None;
}

// returns none if no spell exists with given abbreviation
Spell::Index get_index_by_abbrev(std::string const & spell_abbrev)
{
	// just a linear search, with multiple points of exit.
	unsigned int i = 0;
	while (i < Spell::Count)
	{
		if (s_spell_list[i].abbrev == spell_abbrev)
			return (Spell::Index)i;
		i++;
	}
	return Spell::None;
}

char const * get_description(Spell::Index spell_index)
{
	return "This is a spell."; // todo
//	return spell_description[spell_index];
}

float get_miscast_rate(Spell::Index spell, int skill_magic)
{
	// The miscast rate is based on the difference between difficulty and skill.
	// If difficulty is equal to skill, the miscast rate is 15%.
	// For every 10 points of skill above the difficulty, the rate is halved.
	// For every 10 points of skill below the difficulty, the rate is doubled.

	int difference = Spell::get_difficulty(spell) - skill_magic;

	float mr = 15.0;
	mr = mr * pow(2.0f, difference/10.0f);

	if (mr > 100.0)
		mr = 100.0;

	return mr;
}

static Spell::Instance s_current_spell_instance;

void create_and_bind_instance (Spell::Index spell, int caster)
{
	s_current_spell_instance =
	{
		Spell::get_colour(spell),
		Spell::get_name(spell).at(0),
		Spell::get_damage(spell, caster),
		creature_skill_magic(caster) * Spell::get_difficulty(spell),
		Spell::get_accuracy(spell),
		Spell::get_effect_func(spell)
	};
}

// todo - later we may have a way to create and bind a fake spell
//        for miscasts that create an "unknown spell" effect

Spell::Instance & get_current_instance ()
{
	return s_current_spell_instance;
}

void execute_effect(Spell::Index spell_index, int caster, int target)
{
	Spell::EffectFunc func = s_spell_list[spell_index].effect_func;
	if (func != nullptr)
	{
		func(caster, target);
	}
}

} // namespace Spell
