#include "Potion.h"

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "VectorUtil.h"

#include <array>
#include <format>

namespace Potion
{

//-------------------------------------------------------------------------------------------------
// Potion effect declarations

void drink_wiggenweld(Creature::Handle imbiber);

//-------------------------------------------------------------------------------------------------
// Data

char const* cstr_NoEffect = " The potion has no effect.";

using EffectFunc = void(*)(Creature::Handle imbiber);

struct Info
{
	char const* name;
	char const* colour;
	EffectFunc func;
	char const* description;
};

std::array<Info,Potion::Count> s_potions =
{
	{ "Wiggenweld Potion", "sea", &drink_wiggenweld,
	  "A healing potion prepared with Wiggentree bark." },
};

//-------------------------------------------------------------------------------------------------
// Global interface

int random_flavour()
{
	return Random::in_range(0, (int)Potion::Count);
}

char const* get_name(int potion)
{
	if (is_valid(potion))
	{
		return s_potions[potion].name;
	}
	DebugBreak();
	return "";
}

char const* get_colour(int potion)
{
	if (is_valid(potion))
	{
		return s_potions[potion].colour;
	}
	DebugBreak();
	return "";
}

char const* get_description(int potion)
{
	if (is_valid(potion))
	{
		return s_potions[potion].description;
	}
	DebugBreak();
	return "";
}


void drink (Creature::Handle imbiber, int potion)
{
	if (is_valid(potion) && s_potions[potion].func)
	{
		s_potions[potion].func(imbiber);
	}
}

//-------------------------------------------------------------------------------------------------
// Potion effect implementations

void drink_wiggenweld(Creature::Handle imbiber)
{
	if (imbiber.is_hurt())
	{
		Draw::creature_message(imbiber, std::format(" {} feel your stamina returning.",
			Grammar::You(imbiber)));
		imbiber.heal_hp(8);
	}
	else
	{
		Draw::creature_message(imbiber, cstr_NoEffect);
	}
}

} // namespace Potion
