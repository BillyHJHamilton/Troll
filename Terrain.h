#pragma once

#include "Types.h"

namespace Terrain
{
	enum Type : byte
	{
		Open = 0,
		Wall,
		UpStairs,
		DownStairs
	};

	int get_character(Terrain::Type t);
	bool permits_sight(Terrain::Type t);
	bool is_solid(Terrain::Type t);

}
