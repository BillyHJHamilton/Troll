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
			case Terrain::Chest: return Codepoint::BoxEmpty;
			default: assert(false); return '?';
		}
	}

	const char* get_name(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open: return "floor";
			case Terrain::Wall: return "wall";
			case Terrain::UpStairs: return "stairs";
			case Terrain::DownStairs: return "stairs";
			case Terrain::Chest: return "chest";
			default: assert(false); return "invalid terrain";
		}
	}

	bool permits_sight(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open:
			case Terrain::Chest:
				return true;

			case Terrain::Wall:
			case Terrain::UpStairs:
			case Terrain::DownStairs:
				return false;

			default: assert(false); return false;
		}
	}

	bool is_solid(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open:
			case Terrain::UpStairs:
			case Terrain::DownStairs:
				return false;

			case Terrain::Wall:
			case Terrain::Chest:
				return true;

			default: assert(false); return false;
		}
	}
}
