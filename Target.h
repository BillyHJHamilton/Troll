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
}
