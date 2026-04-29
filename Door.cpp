#include "Door.h"

#include "Debug.h"
#include "Random.h"
#include "Serialize.h"
#include "Terrain.h"

namespace Door
{

void Parameters::serialize(ISerializer& s)
{
	// TODO: Do I need this function?

	for (int i = 0; i < (int)(LockedGenus::Count); i++)
	{
		s.srz_int(locked_genus_weights[i]);
	}
	for (int i = 0; i < (int)(Spelled::Count); i++)
	{
		s.srz_int(spelled_weights[i]);
	}
	for (int i = 0; i < (int)(Triggered::Count); i++)
	{
		s.srz_int(triggered_weights[i]);
	}
	for (int i = 0; i < (int)(TriggerType::Count); i++)
	{
		s.srz_int(trigger_weights[i]);
	}
	for (int i = 0; i < (int)(Unlocked::Count); i++)
	{
		s.srz_int(unlocked_weights[i]);
	}
}

bool Parameters::are_weights_valid() const
{
	int locked_genus_count = 0;
	for (int i = 0; i < (int)(LockedGenus::Count); i++)
	{
		if (locked_genus_weights[i] < 0)
		{
			return false;
		}
		locked_genus_count += locked_genus_weights[i];
	}

	int spelled_count = 0;
	for (int i = 0; i < (int)(Spelled::Count); i++)
	{
		if (spelled_weights[i] < 0)
		{
			return false;
		}
		spelled_count += spelled_weights[i];
	}

	int triggered_count = 0;
	for (int i = 0; i < (int)(Triggered::Count); i++)
	{
		if (triggered_weights[i] < 0)
		{
			return false;
		}
		triggered_count += triggered_weights[i];
	}

	int trigger_type_count = 0;
	for (int i = 0; i < (int)(TriggerType::Count); i++)
	{
		if (trigger_weights[i] < 0)
		{
			return false;
		}
		trigger_type_count += trigger_weights[i];
	}

	int unlocked_count = 0;
	for (int i = 0; i < (int)(Unlocked::Count); i++)
	{
		if (unlocked_weights[i] < 0)
		{
			return false;
		}
		unlocked_count += unlocked_weights[i];
	}

	return
		locked_genus_count > 0 &&
		spelled_count > 0 &&
		triggered_count > 0 &&
		trigger_type_count > 0 &&
		unlocked_count > 0;
}

LockedGenus Parameters::choose_locked_genus(bool allow_none, bool allow_trigger) const
{
	int sum = 0;
	IntTempList weights((int)(LockedGenus::Count), 0);  // count, value

	for (int i = 0; i < Util::Size(weights); ++i)
	{
		if (i == (int)(LockedGenus::None) && !allow_none)
		{
			continue;
		}
		if (i == (int)(LockedGenus::Trigger) && !allow_trigger)
		{
			continue;
		}

		int weight = locked_genus_weights[i];
		weights[i] = weight;
		sum += weight;
	}

	if (sum > 0)
	{
		return (LockedGenus)(Random::weighted_index(weights));
	}

	// no legal door types
	return LockedGenus::None;
}

Spelled Parameters::choose_spelled() const
{
	return (Spelled)(Random::weighted_index(spelled_weights, (int)(Spelled::Count)));
}

Triggered Parameters::choose_triggered() const
{
	return (Triggered)(Random::weighted_index(triggered_weights, (int)(Triggered::Count)));
}

TriggerType Parameters::choose_trigger_type(bool allow_button,
                                            bool allow_torch) const
{
	IntTempList trigger_weights((int)(TriggerType::Count), 0);  // count, value
	int sum = 0;

	if (allow_button)
	{
		int button_weight = trigger_weights[(int)(TriggerType::FlipendoButton)];
		trigger_weights[(int)(TriggerType::FlipendoButton)] = button_weight;
		sum += button_weight;
	}
	if (allow_torch)
	{
		int torch_weight = trigger_weights[(int)(TriggerType::LightTorch)];
		trigger_weights[(int)(TriggerType::LightTorch)] = torch_weight;
		sum += torch_weight;
	}

	if (sum > 0)
	{
		return (TriggerType)(Random::weighted_index(trigger_weights));
	}
	return TriggerType::NotPossible;
}

Unlocked Parameters::choose_unlocked() const
{
	return (Unlocked)(Random::weighted_index(unlocked_weights, (int)(Unlocked::Count)));
}


Terrain::Type get_terrain(Spelled door_type)
{
	switch (door_type)
	{
	case Spelled::Portrait:
		return Terrain::Portrait;
	case Spelled::AlohamoraDoor:
		return Terrain::DoorLocked;
	case Spelled::Ectoplasm:
		return Terrain::Ectoplasm;
	default:
		DebugBreak("Unhandled Spelled door type in get_door_terrain");
		return Terrain::Open;
	}
}

Terrain::Type get_terrain(Triggered door_type)
{
	switch (door_type)
	{
	case Triggered::SlidingWall:
		return Terrain::SlidingWall;
	case Triggered::Portcullis:
		return Terrain::Portcullis;
	default:
		DebugBreak("Unhandled Triggered door type in get_door_terrain");
		return Terrain::Open;
	}
}

Terrain::Type get_terrain(Unlocked door_type)
{
	switch (door_type)
	{
	case Unlocked::None:
		return Terrain::Open;
	case Unlocked::Open:
		return Terrain::DoorOpen;
	case Unlocked::Closed:
		return Terrain::DoorClosed;
	default:
		DebugBreak("Unhandled Unlocked door type in get_door_terrain");
		return Terrain::Open;
	}
}

} // namespace Door
