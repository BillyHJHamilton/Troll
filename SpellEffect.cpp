#include "SpellEffect.h"

#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Status.h"

#include <iostream>
#include <string>

void vermillious_effect (int caster, int target)
{
	std::string message = Grammar::Name_is(target) + " showered in sparks!";
	add_game_message(std::move(message));
}

void flipendo_effect (int caster, int target)
{
	// Push back
	// - Need to get the trajectory of the active spell

	std::string message = Grammar::Name_is(target) + " knocked back!";
	add_game_message(std::move(message));
}

void tarantallegra_effect (int caster, int target)
{
	if (creature_has_status(target, Status::Dancing))
	{
		add_game_message(Grammar::Name_possessive(target) + " feet quicken their dance!");
		inflict_status(target, Status::Dancing, 4);
	}
//	else if (creature_has_status(target, Status::LegLocked))
//	{
//		reduce_status(target, Status::LEG_LOCKED, 4);
//		if (creature_has_status(target, Status::LegLocked))
//		{
//			add_game_message(format_Name_possessive(target) + " legs partially loosen.");		
//		}
//	}
	else
	{
		add_game_message(Grammar::Name_possessive(target) + " feet dance!");
		inflict_status(target, Status::Dancing, 4);
	}
}
