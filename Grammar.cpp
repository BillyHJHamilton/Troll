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
		return creature.short_name();
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
		return creature.short_name();
	}
}

std::string you_pl(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "you";
	}
	else
	{
		switch (creature.gender())
		{
			case Gender::Male:		return "he";
			case Gender::Female:	return "she";
			case Gender::Neuter:
			default:				return "it";
		}
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
		return creature.short_name() + " is";
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
		return creature.short_name() + "\'s";
	}
}

std::string your(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "your";
	}
	else
	{
		return creature.short_name() + "\'s";
	}
}

std::string your_pr(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "your";
	}
	else
	{
		switch (creature.gender())
		{
			case Gender::Male:		return "his";
			case Gender::Female:	return "her";
			case Gender::Neuter:
			default:				return "its";
		}
	}
}

} // namespace Grammar
