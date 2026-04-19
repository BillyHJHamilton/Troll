#include "Sweets.h"

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Player.h"
#include "Random.h"

#include <array>
#include <format>

namespace Sweets
{

//-------------------------------------------------------------------------------------------------
// Data

using EffectFunc = void(*)(Creature::Handle eater);

struct Info
{
	char const* name;
	char const* colour;
	int sugar;
	EffectFunc func;
	char const* description;
};

std::array<Info,Sweets::Count> s_sweets =
{
	Info{ "Pumpkin Pasty", "orange", 4, nullptr,
	  "A turnover pasty with a sweet pumpkin flavour." },
	Info{ "Cauldron Cake", "light cyan", 8, nullptr,
	  "A sugary cake shaped like a cauldron." },
};

std::vector<int> s_weights =
{
	/*PumpkinPasty*/ 5,
	/*CauldronCake*/ 3,
};

//-------------------------------------------------------------------------------------------------
// Global interface

int random_flavour()
{
	return Random::weighted_index(s_weights);
}

char const* get_name(int sweet)
{
	if (is_valid(sweet))
	{
		return s_sweets[sweet].name;
	}
	DebugBreak();
	return "";
}

char const* get_colour(int sweet)
{
	if (is_valid(sweet))
	{
		return s_sweets[sweet].colour;
	}
	DebugBreak();
	return "";
}

char const* get_description(int sweet)
{
	if (is_valid(sweet))
	{
		return s_sweets[sweet].description;
	}
	DebugBreak();
	return "";
}

void eat (Creature::Handle eater, int sweet)
{
	if (eater.is_player())
	{
		Player::gain_sugar(s_sweets[sweet].sugar);
	}

	if (is_valid(sweet) && s_sweets[sweet].func)
	{
		s_sweets[sweet].func(eater);
	}
}

} // namespace Potion
