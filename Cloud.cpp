#include "Cloud.h"
#include "Codepoint.h"
#include "Debug.h"

//int constexpr SOLID_BLOCK = 9608;

namespace Cloud
{
	int get_character(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:
				return Codepoint::BackwardsSquiggle;
			default: DebugBreak(); return '?';
		}
	}

	char const * get_colour(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:		return "grey";
			default: DebugBreak();	return "white";
		}
	}

	int accuracy_loss (Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::None:		return 0;
			case Cloud::Smoke:		return 30;
			default: DebugBreak();	return 0;
		}
	}

	int vision_loss (Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::None:		return 0;
			case Cloud::Smoke:		return 2;
			default: DebugBreak();	return 0;
		}
	}
}