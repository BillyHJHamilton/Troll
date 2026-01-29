#include "Terrain.h"

#include <cassert>

int constexpr SOLID_BLOCK = 9608;

namespace Terrain
{
	int get_character(Terrain::Type t)
	{
		switch (t)
		{
		case Terrain::Open: return '.';
		case Terrain::Wall: return SOLID_BLOCK;
		case Terrain::UpStairs: return '+';
		case Terrain::DownStairs: return '-';
		default: assert(false); return '?';
		}
	}

	bool permits_sight(Terrain::Type t)
	{
		switch (t)
		{
		case Terrain::Open: return true;
		case Terrain::Wall: return false;
		case Terrain::UpStairs: return false;
		case Terrain::DownStairs: return false;
		default: assert(false); return false;
		}
	}

	bool is_solid(Terrain::Type t)
	{
		switch (t)
		{
		case Terrain::Open: return false;
		case Terrain::Wall: return true;
		case Terrain::UpStairs: return false;
		case Terrain::DownStairs: return false;
		default: assert(false); return false;
		}
	}
}
