#include "Spell.h"

#include "BitFlag.h"
#include "Creature.h"
#include "Damage.h"
#include "MiscastCategory.h"
#include "Serialize.h"
#include "SpellEffect.h"
#include "Target.h"

#include <array>

namespace Spell
{

int constexpr c_DmgSP = -2;  // special Stupefy damage--scaled by level

// Note: Spell accuracy is generally 10-20 pts higher than in HPADS, but falls off with range.

// Note: I'd like to use the colour constants from Colour.h, but this is static init'd right now,
// so it's undefined whether the colours are initialized yet.  If only cstrings could be constexpr!

static std::array<Spell::Data, Spell::Count> constexpr s_spell_list = 
{	//			Spell name				Abbrv	Colour				Dif Drk Type			Dmg		Acc Rng	Effect function			Miscast type		Target type		Target flags
	Spell::Data {"Vermillious",			"VM",	"red",				5,	0,	Damage::Fire,	2,		85,	4,	&vermillious,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Flipendo",			"FP",	"orange",			10,	0,	Damage::Basic,	2,		70,	8,	&flipendo,				Miscast::Beam,		Target::Beam,	Target::f_Flipendo },
	Spell::Data {"Alohomora",			"AL",	"light sky",		15,	0,	Damage::None,	0,		50, 8,	&alohomora,				Miscast::Charm,		Target::Beam,	Target::f_Alohomora },
	Spell::Data {"Tarantallegra",		"TA",	"light pink",		15,	0,	Damage::None,	0,		90,	8,	&tarantallegra,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Locomotor Mortis",	"LM",	"yellow",			15,	0,	Damage::None,	0,		90,	8,	&locomotor_mortis,		Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Rictusempra",			"RS",	"light red",		20,	0,	Damage::None,	0,		85,	8,	&rictusempra,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Fumos",				"FM",	"light grey",		25,	0,	Damage::None,	0,		-1, 8,	&fumos,					Miscast::Conjuring,	Target::Beam,	Target::f_Midair },
	Spell::Data {"Mimblewimble",		"MW",	"blue",				25,	0,	Damage::None,	0,		85,	8,	&mimblewimble,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Lacarnum Inflamare",  "LC",   "orange",			25, 0,  Damage::None,	0,		65, 3,  &lacarnum_inflamare,	Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Furnunculus",			"FN",   "lighter orange",	30, 0,  Damage::Basic,	4,		60, 6,  &furnunculus,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Finite Incantatem",	"FI",   "blue",				35, 0,  Damage::None,	0,		-1, 0,  &finite_incantatem,		Miscast::Charm,		Target::Self,	f_None},
	Spell::Data {"Accio",				"AC",   "light sea",		40, 0,  Damage::None,	0,		-1, 8,  &accio,					Miscast::Conjuring, Target::Sight,	Target::f_Midair },
	Spell::Data {"Stupefy",				"SP",   "red",				45, 0,  Damage::Basic,	c_DmgSP,75, 7,  &stupefy,				Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Impedementa",			"IP",   "light green",		45, 0,  Damage::None,	0,		85, 8,  &impedementa,			Miscast::Beam,		Target::Beam,	f_None},
	Spell::Data {"Bat-Bogey Hex",		"BT",   "dark purple",		55, 0,  Damage::None,	0,		80, 6,  &bat_bogey_hex,			Miscast::Beam,		Target::Beam,	f_None},
#if _DEBUG                                                                                                                          
	Spell::Data {"Megabolt",			"MG",	"light amber",		0,	0,	Damage::Basic,	20,		999,8,	nullptr,				Miscast::Beam,		Target::Sight,	f_None},
#else                                                                                                                               
	Spell::Data {"Megabolt",			"MG",	"light amber",		999,0,	Damage::Basic,	0,		0,	0,	nullptr,				Miscast::Beam,		Target::Sight,	f_None },
#endif
};

static std::array<const char*, Spell::Count> constexpr s_spell_description =
{
	/* Vermillious */ "Shoots a shower of hot sparks, dealing minor damage.",
	/* Flipendo */ "The Knockback Jinx.  Knocks the target backwards for small damage.  Deals extra damage if the target hits something.",
	/* Alohomora */ "The Unlocking Charm.  Opens locks on doors and chests.",
	/* Tarantallegra */ "Causes the target\'s feet to dance on their own, which may render him too distracted to cast spells.",
	/* Locomotor Mortis */ "The Leg-Locker Jinx.  Makes the target's legs stick together.  This makes it harder to move, and harder to dodge hostile spells.",
	/* Rictusempra */ "A tickling charm.  Distracts the target and may also cause him to miscast his spells.",
	/* Fumos */ "Produces a cloud of smoke, which reduces line of sight and reduces the accuracy of spells.  Try aiming at a point between you and your enemy before casting.",
	/* Mimblewimble */ "The Tongue-Tying Jynx.  Causes the target to mispronounce his incantations, increasing the chance of spell miscasts.",
	/* Lacarnum Inflamare */ "Used to set fire to the target's clothing, which is highly distracting and also deals some damage each turn.",
	/* Furnunculus */ "Causes the target to burst out in painful boils.",
	/* Finite Incantatem */ "This valuable counter-spell completely ends one enchantment afflicting the caster.",
	/* Accio */ "The Summoning Charm.  Causes an object to fly through the air to the caster.",
	/* Stupefy */ "The Stunning Spell.  This spell deals more damage when used by a more skilled caster.",
	/* Impedementa */ "The Impedement Jinx.  This spell impedes the target's movement, lowering his evasion and causing him to act more slowly.",
	// /* Protego */ "The Shield Charm.  Provides protection against hostile spells, and may deflect a spell back at its caster.  More powerful spells may penetrate the shield."
	/* Bat-Bogey Hex */ "Causes a swarm of black winged things to descend on the target.  This is highly distracting, but also makes the victim somewhat harder to hit.",
	// /* Avis */ "Conjures one or more birds with shark beaks and claws.  The Oppugno spell is required to make the birds attack."
	// /* Oppogno */ "If you have previously summoned birds, this causes them to attack."
	// /* Episky */ "A simple healing spell.  The target regains one or two hitpoints."
	// /* Confundo */ "The Confundus Charm.  The target becomes confused, rendering him less able to aim, dodge, and cast spells."
	// /* Disillusionment */ "The Disillusionment Charm.  Makes the caster hard to see.  This spell is more effective in the hands of a more skilled caster."
	// /* Apparition */ "Apparition allows one to disappear and instantly reappear elsewhere. This may mean a great bonus to accuracy and evasion for the caster while his opponent figures out where he is now."
	// /* Incarcerous */ "Binds the target with thick ropes, which greatly lower his ability to dodge and cast spells, and may also deal damage by constriction."
	// /* Salvio Hexia */ "Sustains defensive spells (such as Protego), preventing them from collapsing on their own while the spell is in effect."
	// /* Extrasensory */ "Sharpens the caster's senses far beyond their natural level, alloweing his spells to be much more accurate."
	// /* Obliviate */ "The memory charm.  May cause the target to forget a spell."
	// /* Wand Arrows */ "Fires a shower of arrows towards the opponent.  Each arrow that hits deals 2 damage."
	// /* Disintegration */ "The user is protected by a powerful magical barrier, which reduces any physical object passing through to dust.  Spells, however, can pass through the barrier unimpeded."
	// /* Watertrap */ "The target is trapped in a crushing sphere of water."
	// /* Crucio */ "The Cruciatus Curse.  Causes the victim extreme pain.  Using this curse is punishable by a lifetime in Azkaban."
	// /* Sectumsempra */ "The victim of this evil spell is slashed open, as if by a sword.  This wound will cause some damage each turn."
	// /* Avada Kedavra */ "The Killing Curse.  Causes a green flash of light, followed by death.  No magical protection can block this spell.  Using this curse is punishable by a lifetime in Azkaban."
	/* Megabolt */ "For Emergency Use Only.  Terms And Conditions May Apply.  Use At Your Own Risk.",
};

bool is_valid_index(Spell::Index index)
{
	return index > Spell::None && index < Spell::Count;
}

std::string get_name (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].name;
}

std::string get_abbrev (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].abbrev;
}

char const* get_colour (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].colour;
}

int get_difficulty (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].difficulty;
}

int get_damage (Spell::Index spell_index, Creature::Handle caster)
{
	assert(is_valid_index(spell_index));

	int const skill_magic = caster.skill_magic();

	if (s_spell_list[spell_index].damage >= 0)
		return s_spell_list[spell_index].damage;
	else if (s_spell_list[spell_index].damage == c_DmgSP)
		return 2 + (skill_magic/20); // (integer division)
	else
		return 0;
}

bool is_damaging (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].damage != 0;
}

Damage::Type damage_type (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].damage_type;
}

Damage::Packet damage_packet (Spell::Index spell_index, Creature::Handle caster)
{
	return Damage::Packet
	{
		.amount = get_damage(spell_index, caster),
		.type = damage_type(spell_index),
		.cause = Damage::Cause(caster)
	};
}

int get_power (Spell::Index spell_index, Creature::Handle caster)
{
	assert(is_valid_index(spell_index));
	return caster.skill_magic() * Spell::get_difficulty(spell_index);
}

Target::Type get_target_type (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].target_type;
}

uint get_target_flags(Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].target_flags;
}

bool has_accuracy (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list.at(spell_index).accuracy > 0
		&& s_spell_list.at(spell_index).accuracy < 100;
}

int get_accuracy (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].accuracy;
}

int get_range(Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].range;
}

bool in_range (Spell::Index index, Vec3 origin, Vec3 target)
{
	assert(is_valid_index(index));

	switch (get_target_type(index))
	{
		case Target::Self:
			return true;

		case Target::Melee:
			return chessboard_adjacent(origin.xy(), target.xy());

		case Target::Beam:
		case Target::Sight:
			return rounded_range(origin.xy(), target.xy(), get_range(index));

		default:
			DebugBreak();
			return false;
	}
}

EffectFunc get_effect_func (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].effect_func;
}

int Spell::get_dark (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].dark;
}

Miscast::Category get_miscast_category (Spell::Index spell_index)
{
	assert(is_valid_index(spell_index));
	return s_spell_list[spell_index].miscast_category;
}

// returns none if no spell exists with given name
Spell::Index get_index_by_name (std::string const & spell_name)
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
Spell::Index get_index_by_abbrev (std::string const & spell_abbrev)
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

char const * get_description (Spell::Index spell_index)
{
	return s_spell_description[spell_index];
}

float get_miscast_rate (Spell::Index spell, int skill_magic)
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

Spell::TempList bitset_to_temp_list(Spell::Bitset const& bitset)
{
	Spell::TempList out_list;
	out_list.reserve(bitset.count());

	for (int i = 0; i < Spell::Index::Count; i++)
	{
		if (bitset.test(i))
		{
			out_list.push_back(static_cast<Spell::Index>(i));
		}
	}
	return out_list;
}

void execute_effect(Spell::Index spell_index, Spell::EffectParams params)
{
	Spell::EffectFunc func = s_spell_list[spell_index].effect_func;
	if (func != nullptr)
	{
		func(params);
	}
}

} // namespace Spell
