#pragma once

#include "Types.h"

namespace Sweets
{
	enum Type : int
	{
		None = c_Invalid,
		PumpkinPasty,
		CauldronCake,
		Count
	};

	inline bool is_valid(Sweets::Type sweet) { return sweet > Sweets::None && sweet < Sweets::Count; }
	inline bool is_valid(int sweet) { return is_valid((Sweets::Type)sweet); }

	int random_flavour();

	char const* get_name(int sweet);
	char const* get_colour(int sweet);
	char const* get_description(int sweet);
	
	int buy_price(int sweet);

	void eat (Creature::Handle eater, int sweet);
}

// "...Fizzing Whizzbees, the levitating sherbet balls that Ron had mentioned;
// along yet another wall were ‘Special Effects’ sweets:
// Drooble’s Best Blowing Gum (which filled a room with bluebell-coloured bubbles that refused to pop for days),
// the strange, splintery Toothflossing Stringmints,
// tiny black Pepper Imps (‘breathe fire for your friends!’),
// Ice Mice (‘hear your teeth chatter and squeak!’),
// peppermint creams shaped like toads (‘hop realistically in the stomach!’),
// fragile sugar-spun quills and exploding bonbons."

// For a good list see: https://harrypotter.fandom.com/wiki/Honeydukes
