#include "Grammar.h"

#include "Creature.h"

namespace Grammar
{

std::string You (int creature_index)
{
	if (creature_is_player(creature_index))
	{
		return "You";
	}
	else
	{
		return creature_name(creature_index);
	}
}

std::string you (int creature_index)
{
	if (creature_is_player(creature_index))
	{
		return "you";
	}
	else
	{
		return creature_name(creature_index);
	}
}

std::string You_are (int creature_index)
{
	if (creature_is_player(creature_index))
	{
		return "You are";
	}
	else
	{
		return creature_name(creature_index) + " is";
	}
}

std::string verbs(std::string verb, int creature_index)
{
	if (creature_is_player(creature_index))
	{
		return verb;
	}
	else
	{
		return verb + "s";
	}
}

std::string Your(int creature_index)
{
	if (creature_is_player(creature_index))
	{
		return "Your";
	}
	else
	{
		return creature_name(creature_index) + "\'s";
	}
}

} // namespace Grammar