#pragma once

#include "Types.h"

namespace Terrain
{
	// Use byte for Terrain::Type to save memory in map arrays.

	enum Type : byte
	{
		Open = 0,
		Wall,
		UpStairs,
		DownStairs,
		Chest
	};

	int get_character(Terrain::Type t);
	char const* get_name(Terrain::Type t);
	bool permits_sight(Terrain::Type t);
	bool is_solid(Terrain::Type t);
}
