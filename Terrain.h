#pragma once

#include "Types.h"
#include <string>

namespace Terrain
{
	// Use byte for Terrain::Type to save memory in map arrays.

	enum Type : byte
	{
		Open = 0,
		OpenNoSpawn,
		OpenHighlight,  // used for debugging
		Wall,
		UpStairs,
		DownStairs,

		// Following types are Features and should be placed using Feature::spawn.
		Chest,
		Portrait,
		FlipendoButton,
		SlidingWall,

		Count
	};

	inline bool is_valid_type(Terrain::Type t) { return t < Count; }

	int get_character(Terrain::Type t);
	char const* get_name(Terrain::Type t);
	std::string look_describe(Terrain::Type t);
	bool permits_sight(Terrain::Type t);
	bool is_can_spawn(Terrain::Type t);
	bool is_solid(Terrain::Type t);
	bool is_stairs(Terrain::Type t);
	bool is_feature(Terrain::Type t);
	bool is_matching_target(Terrain::Type t, uint target_flags);
	Terrain::Type swap_stairs(Terrain::Type t);

	enum class HighlightType : byte
	{
		None = 0,
		Regions,
		Suggestions,
	};

	constexpr HighlightType c_HighlightType = HighlightType::None;
}
