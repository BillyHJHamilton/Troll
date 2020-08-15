#pragma once

#include <string>

namespace Grammar
{
	std::string You (int creature_index);
	std::string you (int creature_index);
	std::string You_are (int creature_index);
	std::string Your (int creature_index);
	std::string verbs (std::string verb, int creature_index);
}