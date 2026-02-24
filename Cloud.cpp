#include "Cloud.h"
#include "Codepoint.h"
#include "Colour.h"
#include "Debug.h"

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
			case Cloud::Smoke:		return cstr_Grey;
			default: DebugBreak();	return cstr_White;
		}
	}

	char const * look_describe(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:		return "- a cloud of smoke";
			default: DebugBreak();	return "- error: unknown cloud type";
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