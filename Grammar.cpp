#include "Grammar.h"

#include "Creature.h"

namespace Grammar
{

std::string You (Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "You";
	}
	else
	{
		return creature.name();
	}
}

std::string you (Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "you";
	}
	else
	{
		return creature.name();
	}
}

std::string You_are (Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "You are";
	}
	else
	{
		return creature.name() + " is";
	}
}

std::string verbs(std::string verb, Creature::Handle creature)
{
	if (creature.is_player())
	{
		return verb;
	}
	else
	{
		return verb + "s";
	}
}

std::string Your(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "Your";
	}
	else
	{
		return creature.name() + "\'s";
	}
}

} // namespace Grammar
