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
	uint constexpr f_CrosshairFill	= 1 << 6;	// Crosshair highlights entire glyph

	struct Data
	{
		char const* name;
		int codepoint;
		int cover_percent;
		int terrain_flags = f_None;
		int target_flags = f_None;
	};

	Terrain::Data const s_data[] = 
	{
		// Remember: These names are used by look_describe and for spell messages.

		//	 Name					Codepoint					Cover	Terrain flags							Target flags
		Data{"floor",				'.',						0,		f_PermitSight | f_CanSpawn,				f_None},
		Data{"floor (highlight)",	':',						0,		f_PermitSight | f_CanSpawn,				f_None},
		Data{"floor (no spawn)",	'.',						0,		f_PermitSight,							f_None},
		Data{"placeholder",			'X',						0,		f_PermitSight,							f_None},
		Data{"wall",				Codepoint::SolidBlock,		100,	f_Solid | f_CrosshairFill,				f_None},
		Data{"up stairs",			Codepoint::CaretUp,			0,		f_Stairs,								f_None},
		Data{"down stairs",			Codepoint::CaretDown,		0,		f_Stairs,								f_None},
		Data{"chest",				Codepoint::Chest,			40,		f_PermitSight | f_Solid | f_Feature,	Target::f_Alohomora},
		Data{"armour",				Codepoint::Armour,			30,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"desk",				Codepoint::Desk1,			50,		f_PermitSight | f_Solid | f_Feature |
																		f_NoAutotarget,							Target::f_Fire | Target::f_Flipendo},
		Data{"torch" /*unlit*/,		Codepoint::TorchUnlit,		25,		f_PermitSight | f_Solid | f_Feature,	Target::f_Fire},
		Data{"torch" /*lit*/,		Codepoint::TorchLit,		25,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"floor" /* scanner */,	'.',						0,		f_PermitSight | f_Feature,				f_None},
		Data{"portrait",			Codepoint::Portrait,		100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Alohomora},
		Data{"button",				Codepoint::FlipendoButton,	100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Flipendo},
		Data{"wall" /*sliding*/,	Codepoint::SolidBlock,		100,	f_Solid | f_Feature | f_CrosshairFill,	f_None},
		Data{"portcullis",			'#',						40,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"floor" /*shop seed*/,	'/',						0,		f_PermitSight | f_Feature,				f_None},
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
			case Terrain::Open:
			case Terrain::OpenNoSpawn:
			case Terrain::OpenHighlight:
				return "- the floor";
			case Terrain::Wall:
			case Terrain::SlidingWall:
				return "- the wall";
			case Terrain::UpStairs: return "- stairs leading up";
			case Terrain::DownStairs: return "- stairs leading down";
			case Terrain::Chest: return "- a locked chest";
			case Terrain::Armour: return "- a suit of armour";
			case Terrain::FlipendoButton: return "- a button on the wall";
			default:
				return std::string("- a ") + get_name(t);
		}
	}

	bool permits_sight(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_PermitSight);
	}

	bool can_spawn(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_CanSpawn);
	}

	bool is_solid(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_Solid);
	}

	int get_cover_percent(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].cover_percent;
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

	bool fills_crosshair(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return Util::IsFlagSet(s_data[t].terrain_flags, f_CrosshairFill);
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
