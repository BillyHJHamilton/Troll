#include "Terrain.h"

#include "BitFlag.h"
#include "Codepoint.h"
#include "Colour.h"
#include "Geometry.h"
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
		char const* colour;
		int cover_percent;
		int terrain_flags = f_None;
		int target_flags = f_None;
	};

	Terrain::Data const s_data[] = 
	{
		// Remember: These names are used by look_describe and for spell messages.

		//	 Name					Codepoint					Colour			Cover	Terrain flags							Target flags
		Data{"floor",				'.',						nullptr,		0,		f_PermitSight | f_CanSpawn,				f_None},
		Data{"floor (highlight)",	':',						nullptr,		0,		f_PermitSight | f_CanSpawn,				f_None},
		Data{"floor (no spawn)",	'.',						nullptr,		0,		f_PermitSight,							f_None},
		Data{"placeholder",			'X',						nullptr,		0,		f_PermitSight,							f_None},
		Data{"wall",				Codepoint::SolidBlock,		nullptr,		100,	f_Solid | f_CrosshairFill,				f_None},
		Data{"up stairs",			Codepoint::CaretUp,			nullptr,		0,		f_Stairs,								f_None},
		Data{"down stairs",			Codepoint::CaretDown,		nullptr,		0,		f_Stairs,								f_None},
		Data{"chest",				Codepoint::Chest,			nullptr,		40,		f_PermitSight | f_Solid | f_Feature,	Target::f_Alohomora},
		Data{"armour",				Codepoint::Armour,			nullptr,		30,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"desk",				Codepoint::Desk1,			nullptr,		50,		f_PermitSight | f_Solid | f_Feature |
																						f_NoAutotarget,							Target::f_Fire | Target::f_Flipendo},
		Data{"torch" /*unlit*/,		Codepoint::TorchUnlit,		nullptr,		25,		f_PermitSight | f_Solid | f_Feature,	Target::f_Fire},
		Data{"torch" /*lit*/,		Codepoint::TorchLit,		nullptr,		25,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"floor" /*pres plate*/,'.',						nullptr,		0,		f_PermitSight | f_Feature,				f_None},
		Data{"floor" /*tripwire X*/,'-',						nullptr,		0,		f_PermitSight | f_CanSpawn | f_Feature,	f_None},
		Data{"floor" /*tripwire Y*/,'|',						nullptr,		0,		f_PermitSight | f_CanSpawn | f_Feature,	f_None},
		Data{"button",				Codepoint::FlipendoButton,	nullptr,		100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Flipendo},
		Data{"portrait",			Codepoint::Portrait,		nullptr,		100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Alohomora},
		Data{"ectoplasm",			Codepoint::EctoplasmDoor,	cstr_LightGreen,100,	f_PermitSight | f_Solid | f_Feature |
																						f_CrosshairFill,						Target::f_Skurge},
		Data{"door", /*open*/		Codepoint::DoorOpen,		nullptr,		0,		f_PermitSight | f_Feature |
																						f_CrosshairFill | f_NoAutotarget,		Target::f_Colloportus},
		Data{"door", /*closed*/		Codepoint::DoorClosed,		nullptr,		100,	f_Feature | f_CrosshairFill |
																						f_NoAutotarget,							Target::f_Colloportus},
		Data{"door", /*locked*/		Codepoint::DoorLocked,		nullptr,		100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Alohomora},
		Data{"door", /*colloportus*/Codepoint::DoorColloportus,	nullptr,		100,	f_Solid | f_Feature | f_CrosshairFill,	Target::f_Alohomora},
		Data{"wall" /*sliding*/,	Codepoint::SolidBlock,		nullptr,		100,	f_Solid | f_Feature | f_CrosshairFill,	f_None},
		Data{"portcullis",			'#',						nullptr,		40,		f_PermitSight | f_Solid | f_Feature,	f_None},
		Data{"floor" /*port tarp*/,	'.',						nullptr,		0,		f_PermitSight | f_Feature,				f_None},
		Data{"floor" /*monstr trp*/,'.',						nullptr,		0,		f_PermitSight | f_Feature,				f_None},
		Data{"floor" /*mon trp am*/,'.',						nullptr,		0,		f_PermitSight | f_Feature,				f_None},
		Data{"floor" /*dead mon*/,	'.',						nullptr,		0,		f_PermitSight | f_CanSpawn | f_Feature,	f_None},
		Data{"floor" /*trgr delay*/,'.',						nullptr,		0,		f_PermitSight | f_CanSpawn | f_Feature,	f_None},
		Data{"floor" /*shop seed*/,	'.',						nullptr,		0,		f_PermitSight | f_Feature,				f_None},
	};

	int get_character(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].codepoint;
	}

	const char* get_colour(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].colour;
	}

	const char* get_name(Terrain::Type t)
	{
		assert(is_valid_type(t));
		return s_data[t].name;
	}

	std::string look_describe(Terrain::Type t)
	{
		if (get_character(t) == get_character(Terrain::Open))
		{
			return "- the floor";
		}

		switch (t)
		{
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
			case Terrain::DoorOpen: return "- an open door";
			case Terrain::DoorClosed: return "- a closed door (unlocked)";
			case Terrain::DoorLocked: return "- a locked door";
			case Terrain::DoorColloportus: return "- a closed door (locked by colloportus)";
			case Terrain::Ectoplasm: return "- ectoplasm";
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

	Terrain::Type get_tripwire(Axis axis)
	{
		assert(axis == c_AxisX || axis == c_AxisY);
		return (axis == c_AxisX) ? TripwireX : TripwireY;
	}
}
