#include "Terrain.h"

#include "BitFlag.h"
#include "Codepoint.h"

#include <cassert>

namespace Terrain
{
	// Terrain flags
	uint constexpr f_PermitSight	= 1 << 0;
	uint constexpr f_Open			= 1 << 1;	// Valid for spawning creatures and features
	uint constexpr f_Solid			= 1 << 2;	// Blocks movement and beams
	uint constexpr f_Stairs			= 1 << 3;
	uint constexpr f_Feature		= 1 << 4;	// Has extra data in Feature module

	struct Data
	{
		char const* name;
		int codepoint;
		int flags = f_None;
	};

	Terrain::Data const s_data[] = 
	{
		Data{"Open",			'.',					f_PermitSight | f_Open},
		Data{"OpenAlternate",	':',					f_PermitSight | f_Open},
		Data{"Wall",			Codepoint::SolidBlock,	f_Solid},
		Data{"UpStairs",		Codepoint::CaretUp,		f_Stairs},
		Data{"DownStairs",		Codepoint::CaretDown,	f_Stairs},
		Data{"Chest",			Codepoint::Chest,		f_PermitSight | f_Solid | f_Feature},
	};

	int get_character(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].codepoint;
	}

	const char* get_name(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].name;
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
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].flags, f_PermitSight);
	}

	bool is_open(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].flags, f_Open);
	}

	bool is_solid(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].flags, f_Solid);
	}

	bool is_stairs(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].flags, f_Stairs);
	}

	bool is_feature(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].flags, f_Feature);
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
