#include "Terrain.h"

#include "BitFlag.h"
#include "Codepoint.h"
#include "Target.h"

#include <cassert>

namespace Terrain
{
	// Terrain flags
	uint constexpr f_PermitSight	= 1 << 0;
	uint constexpr f_CanSpawn		= 1 << 1;	// Valid for spawning creatures and features
	uint constexpr f_Solid			= 1 << 2;	// Blocks movement and beams
	uint constexpr f_Stairs			= 1 << 3;
	uint constexpr f_Feature		= 1 << 4;	// Has extra data in Feature module
	uint constexpr f_NoAutotarget	= 1 << 5;	// Never automatically target

	struct Data
	{
		char const* name;
		int codepoint;
		int terrain_flags = f_None;
		int target_flags = f_None;
	};

	Terrain::Data const s_data[] = 
	{
		// Remember: These names are used by look_describe and for spell messages.

		//	 Name					Codepoint					Terrain flags							Target flags
		Data{"floor",				'.',						f_PermitSight | f_CanSpawn,				f_None},
		Data{"floor (no spawn)",	'.',						f_PermitSight,							f_None},
		Data{"floor (highlight)",	':',						f_PermitSight | f_CanSpawn,				f_None},
		Data{"wall",				Codepoint::SolidBlock,		f_Solid,								f_None},
		Data{"up stairs",			Codepoint::CaretUp,			f_Stairs,								f_None},
		Data{"down stairs",			Codepoint::CaretDown,		f_Stairs,								f_None},
		Data{"chest",				Codepoint::Chest,			f_PermitSight | f_Solid | f_Feature,	Target::f_Alohomora},
		Data{"desk",				Codepoint::Desk1,			f_PermitSight | f_Solid | f_Feature | f_NoAutotarget,	Target::f_Fire | Target::f_Flipendo},
		Data{"torch",				Codepoint::TorchUnlit,		f_PermitSight | f_Solid | f_Feature,	Target::f_Fire}, // unlit
		Data{"torch",				Codepoint::TorchLit,		f_PermitSight | f_Solid | f_Feature,	f_None}, // lit
		Data{"portrait",			Codepoint::Portrait,		f_Solid |  f_Feature,					Target::f_Alohomora},
		Data{"button",				Codepoint::FlipendoButton,	f_Solid |  f_Feature,					Target::f_Flipendo},
		Data{"wall",				Codepoint::SolidBlock,		f_Solid |  f_Feature,					f_None}, // sliding wall
		Data{"portcullis",			'#',						f_PermitSight | f_Solid | f_Feature,	f_None},
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
			case Terrain::Portrait: return "- a portrait";
			case Terrain::FlipendoButton: return "- a button on the wall";
			default:
				return std::string("- the ") + get_name(t);
		}
	}

	bool permits_sight(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_PermitSight);
	}

	bool is_can_spawn(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_CanSpawn);
	}

	bool is_solid(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_Solid);
	}

	bool is_stairs(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_Stairs);
	}

	bool is_feature(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_Feature);
	}

	bool is_auto_target(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return !Util::IsFlagSet(s_data[t].terrain_flags, f_NoAutotarget);
	}

	bool is_matching_target(Terrain::Type t, uint target_flags)
	{
		assert(is_valid_type(t));
		return (s_data[t].target_flags & target_flags) != 0;
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
