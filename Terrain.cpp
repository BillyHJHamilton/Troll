#include "Terrain.h"
#include "Codepoint.h"

#include <cassert>

namespace Terrain
{
	int get_character(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open: return '.';
			case Terrain::Wall: return Codepoint::SolidBlock;
			case Terrain::UpStairs: return Codepoint::ArrowUp;
			case Terrain::DownStairs: return Codepoint::ArrowDown;
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
