#pragma once

#include "Types.h"

namespace Potion
{
	enum Type : int
	{
		None = c_invalid,
		Wiggenweld,
		Count
	};

	inline bool is_valid(int potion) { return potion > Potion::None && potion < Potion::Count; }

	int random_flavour();
	char const* get_name(int potion);
	char const* get_colour(int potion);
	char const* get_description(int potion);

	void drink (Creature::Handle imbiber, int potion);
}

// Some other possible potions:
// Babbling Beverage
// Beautifying Potion
// Befuddlement Draught
// Blood-Replenishing Potion
// Boil Cure Potion
// Calming Draught
// Draught of Living Death
// Dr. Ubbly's Oblivious Unction (cures brain-tentacle welts)
// Elixer to Induce Euphoria
// Essence of Murtlap (eases wounds)
// Felix Felicis
// Fire Protection Potion
// Hair-Raising Potion
// Hiccuping Solution
// Laughing Potion
// Love Potion
// Mandrake Draught (for de-petrifying)
// Pepperup Potion (cure for common cold)
// Polyjuice Potion
// Regulus Moonshine's Potion for Hags (reduces craving for flesh)
// Shrinking Solution
// Strengthening Solution
// Swelling Solution
// Vertiaserum
// Wideye Potion (prevents sleep)
// Wit-Sharpening Potion (increase magic)
// Wolfsbane Potion

// From trading card game:
// Baneberry Potion (a poison)
// Fungiface Potion (causes fungi to grow on face)
// Manegro Potion (grow a lion's mane... courage?)
