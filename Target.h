#pragma once

#include "Types.h"

namespace Target
{
	enum Type : byte
	{
		Self,
		Melee,
		Beam,
		Sight,
	};

	// Hit channel flags
	uint constexpr f_Midair			= 1 << 0;
	uint constexpr f_Flipendo		= 1 << 1;
	uint constexpr f_Alohomora		= 1 << 2;
	uint constexpr f_Skurge			= 1 << 3;
	uint constexpr f_Colloportus	= 1 << 4;
	uint constexpr f_Fire			= 1 << 5;
}
