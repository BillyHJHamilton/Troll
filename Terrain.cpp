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
			case Terrain::UpStairs: return Codepoint::CaretUp;
			case Terrain::DownStairs: return Codepoint::CaretDown;
			case Terrain::Chest: return Codepoint::Chest;
			case Terrain::OpenAlternate: return ':';
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
			case Terrain::OpenAlternate: return "floor";
			default: assert(false); return "invalid terrain";
		}
	}

	std::string look_describe(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::UpStairs: return "- stairs leading up";
			case Terrain::DownStairs: return "- stairs leading down";
			case Terrain::Chest: return "- a locked chest";
			default:
				return std::string("- the ") + get_name(t);
		}
	}

	bool permits_sight(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open:
			case Terrain::Chest:
			case Terrain::OpenAlternate:
				return true;

			case Terrain::Wall:
			case Terrain::UpStairs:
			case Terrain::DownStairs:
				return false;

			default: assert(false); return false;
		}
	}

	bool is_open(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open:
			case Terrain::OpenAlternate:
				return true;

			default:
				return false;
		}
	}

	bool is_solid(Terrain::Type t)
	{
		switch (t)
		{
			case Terrain::Open:
			case Terrain::UpStairs:
			case Terrain::DownStairs:
			case Terrain::OpenAlternate:
				return false;

			case Terrain::Wall:
			case Terrain::Chest:
				return true;

			default: assert(false); return false;
		}
	}

	bool is_stairs(Terrain::Type t)
	{
		switch(t)
		{
			case Terrain::UpStairs:
			case Terrain::DownStairs:
				return true;

			default:
				return false;
		}
	}

	Terrain::Type swap_stairs(Terrain::Type t)
	{
		switch(t)
		{
			case UpStairs: return DownStairs;
			case DownStairs: return UpStairs;
			default: return t;
		}
	}
}
