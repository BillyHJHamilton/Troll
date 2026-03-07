#include "Grammar.h"

#include "Creature.h"
#include "Debug.h"
#include "Gingerbread.h"

#include <format>

namespace Grammar
{

//-------------------------------------------------------------------------------------------------
// Helper functions

std::string capitalized (std::string s)
{
	if (!s.empty())
	{
		s[0] = toupper(s[0]);
	}
	return s;
}

std::string format_name_internal (bool is_player, bool is_generic, Gender gender,
	char const* name, NameParam param = {})
{
	if (is_player)
	{
		switch (param.mode)
		{
			case NameParam::DefinitePossessive:
			case NameParam::IndefinitePossessive:
			case NameParam::PossessivePronoun:
				return (param.capitalize) ? "Your" : "your";

			case NameParam::ReflexivePronoun:
				return (param.capitalize) ? "Yourself" : "yourself";

			default:
				return (param.capitalize) ? "You" : "you";
		}
	}

	if (is_generic)
	{
		switch (param.mode)
		{
			case NameParam::Definite:
				return (param.capitalize) ?
					std::format("The {}", name) :
					std::format("the {}", name);

			case NameParam::Indefinite:
				return (param.capitalize) ?
					std::format("A {}", name) :
					std::format("a {}", name);

			case NameParam::DefinitePossessive:
				return (param.capitalize) ?
					std::format("The {}\'s", name) :
					std::format("the {}\'s", name);

			case NameParam::IndefinitePossessive:
				return (param.capitalize) ?
					std::format("A {}\'s", name) :
					std::format("a {}\'s", name);

			case NameParam::Plain:
			case NameParam::NominativePronoun:
			case NameParam::AccusativePronoun:
			case NameParam::ReflexivePronoun:
			case NameParam::PossessivePronoun:
				break; // Handled the same as non-generic case below.
		}
	}

	switch (param.mode)
	{
		case NameParam::Definite:
		case NameParam::Indefinite:
		case NameParam::Plain:
			return name;

		case NameParam::DefinitePossessive:
		case NameParam::IndefinitePossessive:
			return std::format("{}\'s", name);

		case NameParam::NominativePronoun:
			switch (gender)
			{
				case Gender::Male: return (param.capitalize) ? "He" : "he";
				case Gender::Female: return (param.capitalize) ? "She" : "she";
				case Gender::Neuter: return (param.capitalize) ? "It" : "it";
			}
			break;

		case NameParam::AccusativePronoun:
			switch (gender)
			{
				case Gender::Male: return (param.capitalize) ? "Him" : "him";
				case Gender::Female: return (param.capitalize) ? "Her" : "her";
				case Gender::Neuter: return (param.capitalize) ? "It" : "it";
			}
			break;

		case NameParam::ReflexivePronoun:
			switch (gender)
			{
				case Gender::Male: return (param.capitalize) ? "Himself" : "himself";
				case Gender::Female: return (param.capitalize) ? "Herself" : "herself";
				case Gender::Neuter: return (param.capitalize) ? "Itself" : "itself";
			}
			break;

		case NameParam::PossessivePronoun:
			switch (gender)
			{
				case Gender::Male: return (param.capitalize) ? "His" : "his";
				case Gender::Female: return (param.capitalize) ? "Her" : "her";
				case Gender::Neuter: return (param.capitalize) ? "Its" : "its";
			}
			break;
	}

	DebugBreak();
	return "Error";
}

//-------------------------------------------------------------------------------------------------
// Interface functions

std::string format_name (Creature::Handle creature, NameParam param)
{
	return format_name_internal (
		creature.is_player(), creature.is_generic(), creature.gender(),
		param.long_name ? creature.long_name() : creature.short_name(), param);
}

std::string format_name_by_type (Creature::Type creature_type, NameParam param)
{
	Gingerbread::Stats stats = Gingerbread::read(creature_type);

	return format_name_internal (
		(creature_type == Creature::Player), stats.identity == c_IdentityGeneric,
		stats.gender, param.long_name ? stats.long_name : stats.short_name, param);
}

std::string You (Creature::Handle creature)
{
	NameParam param {.capitalize = true};
	return format_name(creature, param);
}

std::string you (Creature::Handle creature)
{
	return format_name(creature);
}

std::string Your(Creature::Handle creature)
{
	NameParam param {.capitalize = true, .mode=NameParam::DefinitePossessive};
	return format_name(creature, param);
}

std::string your(Creature::Handle creature)
{
	NameParam param {.mode=NameParam::DefinitePossessive};
	return format_name(creature, param);
}

std::string your_pr(Creature::Handle creature)
{
	NameParam param {.mode=NameParam::PossessivePronoun};
	return format_name(creature, param);
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

} // namespace Grammar
