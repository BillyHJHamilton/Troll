#include "Grammar.h"

#include "Creature.h"

#include <format>

namespace Grammar
{

std::string You (Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "You";
	}
	else if (creature.is_generic())
	{
		return std::format("The {}", creature.short_name());
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
	else if (creature.is_generic())
	{
		return std::format("the {}", creature.short_name());
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
	else if (creature.is_generic())
	{
		return std::format("The {} is", creature.short_name());
	}
	else
	{
		return std::format("{} is", creature.short_name());
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

std::string feel (Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "feel";
	}
	else
	{
		return "looks";
	}
}

std::string Your(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "Your";
	}
	else if (creature.is_generic())
	{
		return std::format("The {}\'s", creature.short_name());
	}
	else
	{
		return std::format("{}\'s", creature.short_name());
	}
}

std::string your(Creature::Handle creature)
{
	if (creature.is_player())
	{
		return "your";
	}
	else if (creature.is_generic())
	{
		return std::format("the {}\'s", creature.short_name());
	}
	else
	{
		return std::format("{}\'s", creature.short_name());
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
