#include "Potion.h"

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Math.h"
#include "Random.h"
#include "Status.h"
#include "VectorUtil.h"

#include <array>
#include <format>

namespace Potion
{

//-------------------------------------------------------------------------------------------------
// Potion effect declarations

void drink_calming (Creature::Handle imbiber);
void drink_wiggenweld (Creature::Handle imbiber);

//-------------------------------------------------------------------------------------------------
// Data

char const* cstr_NoEffect = "The potion has no effect.";

using EffectFunc = void(*)(Creature::Handle imbiber);

struct Info
{
	char const* name;
	char const* colour;
	int difficulty;
	EffectFunc func;
	char const* description;
};

std::array<Info,Potion::Count> s_potions =
{
	Info{ "Calming Draught", "light violet", 10, &drink_calming,
	  "A potion to help settle anxious nerves.  It stops uncontrollable dancing and prevents tickling sensations." },
	Info{ "Wiggenweld Potion", "sea", 20, &drink_wiggenweld,
	  "A healing potion prepared with Wiggentree bark.  It restores up to 8 HP." },
};

//-------------------------------------------------------------------------------------------------
// Global interface

int random_flavour()
{
	return Random::in_range(0, (int)Potion::Count);
}

Potion::Type random_by_level(float target_difficulty)
{
	std::vector<float> weights;
	weights.reserve(Potion::Count);

	float constexpr c_OverLevelFactor = 0.75f;
	float constexpr c_UnderLevelFactor = 0.9f;
	
	for (int i = 0; i < Potion::Count; ++i)
	{
		float weight = 1.0f;
		float const potion_difficulty = ((float)s_potions[i].difficulty) / 10.0f;

		if (Math::FloatGreater(potion_difficulty, target_difficulty))
		{
			float const difference = potion_difficulty - target_difficulty;
			weight *= pow(c_OverLevelFactor, difference);
		}
		else if (Math::FloatLess(potion_difficulty, target_difficulty))
		{
			float const difference = target_difficulty - potion_difficulty;
			weight *= pow(c_UnderLevelFactor, difference);
		}

		weights.push_back(weight);
	}
	
	// In future, when there is more than one potion, we'll choose one based on difficulty.
	return (Potion::Type)Random::weighted_index(weights);
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

int buy_price(int potion)
{
	if (is_valid(potion))
	{
		return s_potions[potion].difficulty - 2;
	}
	return c_Invalid;
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

void drink_calming(Creature::Handle imbiber)
{
	if (imbiber.has_status(Status::Dancing))
	{
		imbiber.cure_status(Status::Dancing);
	}

	if (imbiber.has_status(Status::Tickled))
	{
		imbiber.cure_status(Status::Tickled);
	}

	Draw::creature_message(imbiber, std::format("{} {} suddenly calm.",
			Grammar::You(imbiber), Grammar::feel(imbiber)));
	imbiber.inflict_status(Status::Calm, 9);
}

void drink_wiggenweld(Creature::Handle imbiber)
{
	if (imbiber.is_hurt())
	{
		if (imbiber.is_player())
		{
			Draw::add_message("You feel your stamina returning.");
		}
		else
		{
			Draw::creature_message(imbiber,
				std::format("You see {} stamina returning.",
				Grammar::your(imbiber)));
		}

		imbiber.heal_hp(8);
	}
	else
	{
		Draw::creature_message(imbiber, cstr_NoEffect);
	}
}

} // namespace Potion
