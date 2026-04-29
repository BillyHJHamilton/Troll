#pragma once

#include "Types.h"

namespace Door
{
	enum class LockedGenus : int
	{
		None,
		Spell,
		Trigger,
		Count,
	};

	enum class Spelled : int
	{
		NotPossible = -1,  // error value
		Portrait,
		AlohamoraDoor,
		Ectoplasm,
		Count,
	};

	enum class Triggered : int
	{
		NotPossible = -1,  // error value
		SlidingWall,
		Portcullis,
		Count,
	};

	enum class TriggerType : int
	{
		NotPossible = -1,  // error value
		FlipendoButton,
		LightTorch,
		Count,
	};

	enum class Unlocked : int
	{
		None,
		Open,
		Closed,
		Count,
	};

	Terrain::Type get_terrain(Spelled door_type);
	Terrain::Type get_terrain(Triggered door_type);
	Terrain::Type get_terrain(Unlocked door_type);

} // namespace Door
