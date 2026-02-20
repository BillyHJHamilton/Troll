#pragma once

#include "Types.h"

namespace Terrain
{
	// Using byte here will save a lot of memory in the map arrays.

	enum Type : byte
	{
		Open = 0,
		Wall,
		UpStairs,
		DownStairs,
		Chest
	};

	int get_character(Terrain::Type t);
	bool permits_sight(Terrain::Type t);
	bool is_solid(Terrain::Type t);

}
