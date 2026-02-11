#include "House.h"
#include "Debug.h"

namespace House
{
	char const * name(Id id)
	{
		switch (id)
		{
			case Gryffindor: return "Gryffindor";
			case Hufflepuff: return "Hufflepuff";
			case Ravenclaw:  return "Ravenclaw";
			case Slytherin:  return "Slytherin";
			default: DebugBreak(); return "";
		}
	}

	char const * colour(Id id)
	{
		switch (id)
		{
			case Gryffindor: return "light red";
			case Hufflepuff: return "light yellow";
			case Ravenclaw:  return "light blue";
			case Slytherin:  return "light green";
			default: DebugBreak(); return "";
		}
	}

	char const * description(Id id)
	{
		switch (id)
		{
			case Gryffindor: return
				"Fearless and brave.\n"
				"Start with +1 courage (not a real mechanic yet)";
			case Hufflepuff: return
				"Loyal and hard-working.\n"
				"Start with +2 Hit Points";
			case Ravenclaw:  return
				"Clever and quick-witted.\n"
				"Start with +5 Magic Skill";
			case Slytherin:  return
				"Cunning and ambitious.\n"
				"Can learn dark spells.";
			default: DebugBreak(); return "";
		}
	}

}
