#pragma once

#include "Creature.h"

#include <string>

namespace Grammar
{
	std::string You (Creature::Handle creature);
	std::string you (Creature::Handle creature);
	std::string You_are (Creature::Handle creature);
	std::string Your (Creature::Handle creature);
	std::string verbs (std::string verb, Creature::Handle creature);
}