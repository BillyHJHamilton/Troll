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
	uint constexpr f_Creature		= 1 << 0;
	uint constexpr f_Item			= 1 << 1;
	uint constexpr f_Flipendo		= 1 << 2;
	uint constexpr f_Alohomora		= 1 << 3;
}
