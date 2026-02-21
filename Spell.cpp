#include "Spell.h"

#include "Creature.h"
#include "MiscastCategory.h"
#include "Serialize.h"
#include "SpellEffect.h"

#include <array>

namespace Spell
{

int constexpr c_DmgSP = -2;  // special Stupefy damage--scaled by level

// Note: Spell accuracy is generally 10-20 pts higher than in HPADS.

// Note: I'd like to use the colour constants from Colour.h, but this is static init'd right now,
// so it's undefined whether the colours are initialized yet.  If only cstrings could be constexpr!

static std::array<Spell::Data, Spell::Count> constexpr s_spell_list = 
{	//			Spell name				Abbrv	Colour				Dif Drk Dmg			Acc Rng	Effect function			Target type				Miscast type
	Spell::Data {"Vermillious",			"VM",	"red",				5,	0,	2,			85,	4,	&vermillious,			TargetType::Creature,	Miscast::Beam },
#if d_EnableMegadrill
	Spell::Data {"Megadrill",			"MG",	"light amber",		5,	0,	20,			999,8,	nullptr,				TargetType::Creature,	Miscast::Beam },
#endif
	Spell::Data {"Flipendo",			"FP",	"orange",			10,	0,	2,			70,	8,	&flipendo,				TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Alohomora",			"AL",	"light sky",		15,	0,	0,			50, 8,	&alohomora,				TargetType::Tile,		Miscast::Charm },
	Spell::Data {"Tarantallegra",		"TA",	"light pink",		15,	0,	0,			90,	8,	&tarantallegra,			TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Locomotor Mortis",	"LM",	"yellow",			15,	0,	0,			85,	8,	&locomotor_mortis,		TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Rictusempra",			"RS",	"light red",		20,	0,	0,			90,	8,	&rictusempra,			TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Fumos",				"FM",	"light grey",		25,	0,	0,			-1, 8,	&fumos,					TargetType::Tile,		Miscast::Conjuring },
	Spell::Data {"Mimblewimble",		"MW",	"blue",				25,	0,	0,			90,	8,	&mimblewimble,			TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Lacarnum Inflamare",  "LC",   "orange",			25, 0,  0,			65, 3,  &lacarnum_inflamare,	TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Furnunculus",			"FN",   "lighter orange",	30, 0,  4,			60, 6,  &furnunculus,			TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Finite Incantatem",	"FI",   "blue",				35, 0,  0,			-1, 8,  &finite_incantatem,		TargetType::Self,		Miscast::Charm },
	Spell::Data {"Accio",				"AC",   "light sea",		40, 0,  0,			-1, 8,  &accio,					TargetType::Sight,		Miscast::Conjuring },
	Spell::Data {"Stupefy",				"SP",   "red",				45, 0,  c_DmgSP,	75, 7,  &stupefy,				TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Impedementa",			"IP",   "light green",		45, 0,  0,			85, 8,  &impedementa,			TargetType::Creature,	Miscast::Beam },
	Spell::Data {"Bat-Bogey Hex",		"BT",   "dark purple",		55, 0,  0,			80, 6,  &bat_bogey_hex,			TargetType::Creature,	Miscast::Beam },
};

static std::array<const char*, Spell::Count> constexpr s_spell_description =
{
	/* VERMILLIOUS */ "Shoots a shower of hot sparks, dealing minor damage.",
#if d_EnableMegadrill
	/* MEGADRILL */ "For Emergency Use Only.  Terms And Conditions Apply.  Use At Your Own Risk.",
#endif
	/* FLIPENDO */ "The Knockback Jinx.  Knocks the target backwards for small damage.  Deals extra damage if the target hits something.",
	/* ALOHOMORA */ "The Unlocking Charm.  Opens locks on doors and chests.",
	/* TARANTALLEGRA */ "Causes the target\'s feet to dance on their own, which may render him too distracted to cast spells.",
	/* LOCOMOTOR_MORTIS */ "The Leg-Locker Jinx.  Makes the target's legs stick together.  This makes it harder to move, and harder to dodge hostile spells.",
	/* RICTUSEMPRA */ "A tickling charm.  Distracts the target and may also cause him to miscast his spells.",
	/* FUMOS */ "Produces a cloud of smoke, which reduces line of sight and reduces the accuracy of spells.",
	/* MIMBLEWIMBLE */ "The Tongue-Tying Jynx.  Causes the target to mispronounce his incantations, increasing the chance of spell miscasts.",
	/* LACARNUM_INF */ "Used to set fire to the target's clothing, which is highly distracting and also deals some damage each turn.",
	/* FURNUNCULUS */ "Causes the target to burst out in painful boils.",
	/* ACCIO */ "The Summoning Charm.  Causes an object to fly through the air to the caster.",
	/* STUPEFY */ "The Stunning Spell.  This spell deals more damage when used by a more skilled caster.",
	/* IMPEDEMENTA */ "The Impedement Jinx.  This spell impedes the target's movement, lowering his evasion and causing him to act more slowly.",
	/* FINITE_INC */ "This valuable counter-spell completely ends one enchantment afflicting the caster.",
	// /* PROTEGO */ "The Shield Charm.  Provides protection against hostile spells, and may deflect a spell back at its caster.  More powerful spells may penetrate the shield."
	/* BAT_BOGEY */ "Causes a swarm of black winged things to descend on the target.  This is highly distracting, but also makes the victim somewhat harder to hit.",
	// /* AVIS */ "Conjures one or more birds with shark beaks and claws.  The Oppugno spell is required to make the birds attack."
	// /* OPPUGNO */ "If you have previously summoned birds, this causes them to attack."
	// /* EPISKY */ "A simple healing spell.  The target regains one or two hitpoints."
	// /* CONFUNDO */ "The Confundus Charm.  The target becomes confused, rendering him less able to aim, dodge, and cast spells."
	// /* DISILLUSIONMENT */ "The Disillusionment Charm.  Makes the caster hard to see.  This spell is more effective in the hands of a more skilled caster."
	// /* APPARITION */ "Apparition allows one to disappear and instantly reappear elsewhere. This may mean a great bonus to accuracy and evasion for the caster while his opponent figures out where he is now."
	// /* INCARCEROUS */ "Binds the target with thick ropes, which greatly lower his ability to dodge and cast spells, and may also deal damage by constriction."
	// /* SALVIO_HEXIA */ "Sustains defensive spells (such as Protego), preventing them from collapsing on their own while the spell is in effect."
	// /* EXTRASENSORY */ "Sharpens the caster's senses far beyond their natural level, alloweing his spells to be much more accurate."
	// /* OBLIVIATE */ "The memory charm.  May cause the target to forget a spell."
	// /* WAND_ARROWS */ "Fires a shower of arrows towards the opponent.  Each arrow that hits deals 2 damage."
	// /* DISINTEGRATION */ "The user is protected by a powerful magical barrier, which reduces any physical object passing through to dust.  Spells, however, can pass through the barrier unimpeded."
	// /* WATERTRAP */ "The target is trapped in a crushing sphere of water."
	// /* CRUCIO */ "The Cruciatus Curse.  Causes the victim extreme pain.  Using this curse is punishable by a lifetime in Azkaban."
	// /* SECTUMSEMPRA */ "The victim of this evil spell is slashed open, as if by a sword.  This wound will cause some damage each turn."
	// /* AVADA_KEDAVRA */ "The Killing Curse.  Causes a green flash of light, followed by death.  No magical protection can block this spell.  Using this curse is punishable by a lifetime in Azkaban."
};

void init()
{
	// none required, thank you constexpr
}

void srz_bitset(ISerializer& s, Spell::Bitset& bitset)
{
	std::string str;
	if (s.is_load())
	{
		s.srz_string(str);
		bitset = Spell::Bitset(str);
	}
	else
	{
		str = bitset.to_string();
		s.srz_string(str);
	}
}

bool is_valid_index(Spell::Index index)
{
	return index > Spell::None && index < Spell::Count;
}

std::string get_name (Spell::Index spell_index)
{
	return s_spell_list[spell_index].name;
}

std::string get_abbrev (Spell::Index spell_index)
{
	return s_spell_list[spell_index].abbrev;
}

std::string get_colour (Spell::Index spell_index)
{
	return s_spell_list[spell_index].colour;
}

int get_difficulty (Spell::Index spell_index)
{
	return s_spell_list[spell_index].difficulty;
}

int get_damage (Spell::Index spell_index, Creature::Handle caster)
{
	int const skill_magic = caster.skill_magic();

	if (s_spell_list[spell_index].damage >= 0)
		return s_spell_list[spell_index].damage;
	else if (s_spell_list[spell_index].damage == c_DmgSP)
		return 2 + (skill_magic/20); // (integer division)
	else
		return 0;
}

int is_damaging (Spell::Index spell_index)
{
	return s_spell_list[spell_index].damage != 0;
}

TargetType get_target_type (Spell::Index spell_index)
{
	return s_spell_list[spell_index].target_type;
}

int get_accuracy (Spell::Index spell_index)
{
	return s_spell_list[spell_index].accuracy;
}

int get_range(Spell::Index spell_index)
{
	return s_spell_list[spell_index].range;
}

EffectFunc get_effect_func (Spell::Index spell_index)
{
	return s_spell_list[spell_index].effect_func;
}

int Spell::get_dark (Spell::Index spell_index)
{
	return s_spell_list[spell_index].dark;
}

Miscast::Category get_miscast_category (Spell::Index spell_index)
{
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

static Spell::Instance s_current_spell_instance;

void create_and_bind_instance (Spell::Index spell, Creature::Handle caster)
{
	s_current_spell_instance =
	{
		Spell::get_colour(spell),
		Spell::get_name(spell).at(0),
		Spell::get_damage(spell, caster),
		caster.skill_magic() * Spell::get_difficulty(spell),
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
