#pragma once

#include "Creature.h"

#include <string>

namespace Grammar
{
	struct NameParam
	{
		bool capitalize = false;
		bool long_name = false;

		enum Mode : int
		{
			Definite,				// you / the fire crab / Hermione
			Indefinite,				// you / a fire crab / Hermione
			Plain,					// you / fire crab / Hermione
			DefinitePossessive,		// your / the fire crab's / Hermione's
			IndefinitePossessive,	// your / a fire crab's / Hermione's
			NominativePronoun,		// you / he / she / it
			AccusativePronoun,		// you / him / her / it
			ReflexivePronoun,		// yourself / himself / herself / itself
			PossessivePronoun,		// your / his / her / its
		};
		Mode mode = Definite;
	};
	std::string format_name (Creature::Handle creature, NameParam param = {});
	std::string format_name_by_type (Creature::Type creature_type, NameParam param = {});

	std::string you (Creature::Handle creature);
	std::string You (Creature::Handle creature);
	std::string Your (Creature::Handle creature);
	std::string your (Creature::Handle creature);
	std::string your_pr (Creature::Handle creature); // uses his/her/its instead of name

	std::string You_are (Creature::Handle creature);
	std::string You_have (Creature::Handle creature);
	std::string verbs (std::string verb, Creature::Handle creature);
	std::string feel (Creature::Handle creature);
}
