#include "Door.h"

#include "Debug.h"
#include "Terrain.h"

namespace Door
{

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
