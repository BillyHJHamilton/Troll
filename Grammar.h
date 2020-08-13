#pragma once

#include <string>

namespace Grammar
{
	std::string Name (int creature_index);
	std::string name (int creature_index);
	std::string Name_is (int creature_index);
	std::string Name_possessive(int creature_index);
	std::string verbs(std::string verb, int creature_index);
}