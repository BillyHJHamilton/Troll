#pragma once

#include "Types.h"

static constexpr uint f_None = 0u;

namespace Util
{
	inline void SetFlag(uint& bitset, uint flagToSet) { bitset |= flagToSet; }
	inline void ClearFlag(uint& bitset, uint flagToClear) { bitset &= ~flagToClear; }
	inline bool IsFlagSet(const uint& bitset, uint flagToCheck) { return bitset & flagToCheck; }
}
