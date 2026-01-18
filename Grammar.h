#pragma once

#include "Creature.h"

#include <string>

namespace Grammar
{
	std::string You (Creature::Handle creature);
	std::string you (Creature::Handle creature);
	std::string you_pr (Creature::Handle creature); // uses he/she/it instead of name
	std::string You_are (Creature::Handle creature);
	std::string Your (Creature::Handle creature);
	std::string your (Creature::Handle creature);
	std::string your_pr (Creature::Handle creature); // uses his/her/its instead of name
	std::string verbs (std::string verb, Creature::Handle creature);
}