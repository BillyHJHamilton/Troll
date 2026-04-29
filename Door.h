#pragma once

#include "Types.h"

namespace Door
{
	enum class LockedGenus : int
	{
		None = 0,
		Spell,
		Trigger,
		Count,
	};

	enum class Spelled : int
	{
		NotPossible = -1,  // error value
		Portrait = 0,
		AlohamoraDoor,
		Ectoplasm,
		Count,
	};

	enum class Triggered : int
	{
		NotPossible = -1,  // error value
		SlidingWall = 0,
		Portcullis,
		Count,
	};

	enum class TriggerType : int
	{
		NotPossible = -1,  // error value
		FlipendoButton = 0,
		LightTorch,
		Count,
	};

	enum class Unlocked : int
	{
		None = 0,
		Open,
		Closed,
		Count,
	};

	struct Parameters
	{
		// Type distributions for locked doors
		int locked_genus_weights[(int)(Door::LockedGenus::Count)] = { 1 };  // None: 1, all others: 0
		int spelled_weights[(int)(Door::Spelled::Count)] = { 1 };  // Portrait: 1, all others: 0
		int triggered_weights[(int)(Door::Triggered::Count)] = { 1 };  // SlidingWall: 1, all others: 0
		int trigger_weights[(int)(Door::TriggerType::Count)] = { 1 };  // FlipendoButton: 1, all others: 0

		// Type distribution for unlocked doors
		int unlocked_weights[(int)(Door::Unlocked::Count)] = { 1 };  // None: 1, all others: 0

		void serialize(ISerializer& s);

		bool are_weights_valid() const;  // requires no negatives, at least 1 in each catagory

		Door::LockedGenus choose_locked_genus(bool allow_none, bool allow_trigger) const;
		Door::Spelled choose_spelled() const;
		Door::Triggered choose_triggered() const;
		Door::TriggerType choose_trigger_type(bool allow_button, bool allow_torch) const;

		Door::Unlocked choose_unlocked() const;
	};

	Terrain::Type get_terrain(Spelled door_type);
	Terrain::Type get_terrain(Triggered door_type);
	Terrain::Type get_terrain(Unlocked door_type);

} // namespace Door
