#include "Grammar.h"

#include "Creature.h"

namespace Grammar
{

std::string Name (int creature_index)
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

std::string name (int creature_index)
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

std::string Name_is (int creature_index)
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

std::string Name_possessive(int creature_index)
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