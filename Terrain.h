#pragma once

#include "Types.h"
#include <string>

namespace Terrain
{
	// Use byte for Terrain::Type to save memory in map arrays.

	enum Type : byte
	{
		Open = 0,
		Wall,
		UpStairs,
		DownStairs,
		Chest,
		OpenIsolated,
	};

	int get_character(Terrain::Type t);
	char const* get_name(Terrain::Type t);
	std::string look_describe(Terrain::Type t);
	bool permits_sight(Terrain::Type t);
	bool is_open(Terrain::Type t);
	bool is_solid(Terrain::Type t);
	bool is_stairs(Terrain::Type t);
	Terrain::Type swap_stairs(Terrain::Type t);
}
