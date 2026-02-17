#include "Room.h"

#include "Debug.h"
#include "Map.h"
#include "Math.h"
#include "Random.h"
#include "Terrain.h"

#include <cassert>

// Chamber maker
Room::Room(Box2 box, RoomType roomType)
	: m_RoomType(roomType)
	, m_Box(box)
{
	assert(m_RoomType != RoomType::Corridor);
}

// Corridor maker
Room::Room(Box2 box, Axis corridorAxis)
	: m_RoomType(RoomType::Corridor)
	, m_Box(box)
	, m_CorridorAxis(corridorAxis)
{
	// For now, all corridors must be 1 wide
	assert(m_Box.size.x == 1 || m_Box.size.y == 1);
}

// Stairs maker
Room::Room(Vec2 localEnd, Stairs::Direction StairsDirection)
	: m_RoomType(RoomType::Stairs)
	, m_Box(Stairs::get_box(localEnd, StairsDirection))
	, m_StairsDirection(StairsDirection)
{
}

void Room::Serialize(ISerializer& s)
{
	s.srz_box2(m_Box);
	srz_value(s, m_RoomType);
	srz_value(s, m_StairsDirection);
	s.srz_int(m_CorridorAxis);
}

int Room::CorridorLength() const
{
	if (m_CorridorAxis == AXIS_X)
	{
		return m_Box.size.x;
	}
	else
	{
		return m_Box.size.y;
	}
}

// static
Vec2 Room::StairsLocalEnd() const
{
	switch(m_StairsDirection)
	{
		case Stairs::Direction::DownEast:
		case Stairs::Direction::UpEast:
		case Stairs::Direction::UpSouth:
		case Stairs::Direction::DownSouth:
			return m_Box.min;
		case Stairs::Direction::DownNorth:
		case Stairs::Direction::UpNorth:
			return m_Box.min + Vec2{0,1};
		case Stairs::Direction::DownWest:
		case Stairs::Direction::UpWest:
			return m_Box.min + Vec2{1,0};
		default:
			DebugBreak("Unhandled case");
			return {0,0};
	}
}

Vec2 Room::StairsRemoteEnd() const
{
	return StairsLocalEnd() + Stairs::relative_move(m_StairsDirection).xy();
}

Axis Room::StairsAxis(Stairs::Direction direction)
{
	int const x = Stairs::relative_move(direction).x;
	if (x != 0)
	{
		return AXIS_X;
	}
	else
	{
		return AXIS_Y;
	}
}

bool Room::VetoesRoom(Room const &newRoom) const
{
	Box2 const &newBox = newRoom.GetBox();

	if (!newBox.intersects_or_adjacent(m_Box))
	{
		return false;
	}

	// OK, so we're adjacent, huh?
	if (newRoom.JoinsToRoom(*this) || JoinsToRoom(newRoom))
	{
		// that's ok then
		return false;
	}
	else
	{
		// Extremely Problematic
		return true;
	}
}

// Stairs or corridors
bool Room::JoinsToRoom(Room const &room) const
{
	switch(m_RoomType)
	{
		case RoomType::Stairs: return JoinsToRoomAsStairs(room);
		case RoomType::Corridor: return JoinsToRoomAsCorridor(room);
		default: return false;
	}
}

bool Room::JoinsToRoomAsCorridor(Room const &room) const
{
	Axis overlapAxis = get_other_axis(m_CorridorAxis);

	if (!m_Box.overlaps_on_axis(room.GetBox(), get_other_axis(m_CorridorAxis)))
	{
		return false;
	}

	return (m_Box.min[m_CorridorAxis] == room.GetBox().min[m_CorridorAxis] - m_Box.size[m_CorridorAxis])
		|| (m_Box.min[m_CorridorAxis] == room.GetBox().max()[m_CorridorAxis]);
}

bool Room::JoinsToRoomAsStairs(Room const &room) const
{
	// Stairs only join on one side.
	Vec2 joiningPlace = StairsLocalEnd() + Stairs::joining_vector(m_StairsDirection);

	return room.GetBox().contains(joiningPlace)
		&& !room.GetBox().contains(StairsLocalEnd());
}

std::vector<Room> Room::FindPossibleJoiningCorridors(Room const & other) const
{
	std::vector<Room> output;

	if (m_Box.intersects_or_adjacent(other.GetBox()))
	{
		return output;
	}
	
	if (other.m_RoomType == RoomType::Stairs)
	{
		return output;
	}

	// Special case for stairs
	if (m_RoomType == RoomType::Stairs)
	{
		return FindPossibleJoiningCorridorsAsStairs(other);
	}

	// See in what manner we might join the rooms
	Axis overlapAxis;
	if (m_Box.overlaps_on_axis(other.GetBox(), AXIS_X))
	{
		overlapAxis = AXIS_X;
	}
	else if (m_Box.overlaps_on_axis(other.GetBox(), AXIS_Y))
	{
		overlapAxis = AXIS_Y;
	}
	else
	{
		return output;
	}

	Axis corridorAxis = get_other_axis(overlapAxis);

	int minPos = std::max(m_Box.min[overlapAxis], other.GetBox().min[overlapAxis]);
	int maxPos = std::min(m_Box.max()[overlapAxis], other.GetBox().max()[overlapAxis]);

	for (int pos = minPos; pos <= maxPos; pos++)
	{
		output.push_back(FindPossibleJoiningCorridorCommon(other, corridorAxis, pos));
	}

	return output;
}

Room Room::FindPossibleJoiningCorridorCommon(Room const &other, Axis corridorAxis, int posOnOtherAxis) const
{
	Axis overlapAxis = get_other_axis(corridorAxis);

	// Make this a helper function
	if (other.GetBox().min[corridorAxis] > m_Box.min[corridorAxis])
	{
		// It's on the positive side
		Box2 corridorBox;
		corridorBox.min[corridorAxis] = m_Box.max()[corridorAxis];
		corridorBox.size[corridorAxis] = other.GetBox().min[corridorAxis] - corridorBox.min[corridorAxis];
		corridorBox.min[overlapAxis] = posOnOtherAxis;
		corridorBox.size[overlapAxis] = 1;
		return MakeCorridor(corridorBox, corridorAxis);
	}
	else
	{
		// It's on the negative side
		Box2 corridorBox;
		corridorBox.min[corridorAxis] = other.GetBox().max()[corridorAxis];
		corridorBox.size[corridorAxis] = m_Box.min[corridorAxis] - corridorBox.min[corridorAxis];
		corridorBox.min[overlapAxis] = posOnOtherAxis;
		corridorBox.size[overlapAxis] = 1;
		return MakeCorridor(corridorBox, corridorAxis);
	}
}

std::vector<Room> Room::FindPossibleJoiningCorridorsAsStairs(Room const & other) const
{
	std::vector<Room> output;

	Axis corridorAxis = StairsAxis(m_StairsDirection);
	Axis overlapAxis = get_other_axis(corridorAxis);
	if (!m_Box.overlaps_on_axis(other.GetBox(), overlapAxis))
	{
		return output;
	}

	// They overlap, but is it on the correct side?
	int requiredSign = Stairs::joining_vector(m_StairsDirection)[corridorAxis];
	int roomDiff = other.GetBox().min[corridorAxis] - m_Box.min[corridorAxis];
	if (!Math::SameSign(requiredSign, roomDiff))
	{
		return output;
	}

	// Find the one possible corridor and return it.
	int pos = StairsLocalEnd()[overlapAxis];
	output.push_back(FindPossibleJoiningCorridorCommon(other, corridorAxis, pos));
	return output;
}

Vec2 Room::SuggestRandAdjoiningPositionForRoom(Vec2 roomSize) const
{
	if (m_RoomType != RoomType::Stairs)
	{
		DebugBreak("You are only supposed to use this for stairs.");
		return Vec2{0,0};
	}

	Vec2 joinPos = StairsLocalEnd() + Stairs::joining_vector(m_StairsDirection);

	Vec2 output;

	// Find the near side and far side of the room, and return the min of those
	// since that is where the origin-corner will be.
	Axis stairsAxis = StairsAxis(m_StairsDirection);
	int nearSide = joinPos[stairsAxis];
	int farSide = nearSide
		+ (roomSize[stairsAxis]-1) * Stairs::joining_vector(m_StairsDirection)[stairsAxis];
	output[stairsAxis] = std::min(nearSide, farSide);

	// Pick a random place side to side
	Axis slidingAxis = get_other_axis(stairsAxis);
	int minPos = joinPos[slidingAxis] - roomSize[slidingAxis] + 1;
	int maxPos = joinPos[slidingAxis];
	output[slidingAxis] = Random::in_range(minPos, maxPos);

	return output;
}

std::vector<Room> Room::FindPossibleJoiningStairs(bool goingUp) const
{
	std::vector<Room> output;

	// only put them on chambers for now
	if (!IsChamber())
	{
		return output;
	}

	// top and bottom
	for (int pos = m_Box.min.x; pos < m_Box.max(AXIS_X); ++pos)
	{
		output.push_back(MakeStairs(Vec2{pos, m_Box.min.y - 1},
			goingUp ? Stairs::UpNorth : Stairs::DownNorth));
		output.push_back(MakeStairs(Vec2{pos, m_Box.max(AXIS_Y)},
			goingUp ? Stairs::UpSouth : Stairs::Direction::DownSouth));
	}
	// left and right
	for (int pos = m_Box.min.y; pos < m_Box.max(AXIS_Y); ++pos)
	{
		output.push_back(MakeStairs(Vec2{m_Box.min.x - 1, pos},
			goingUp ? Stairs::UpWest : Stairs::DownWest));
		output.push_back(MakeStairs(Vec2{m_Box.max(AXIS_X), pos},
			goingUp ? Stairs::UpEast : Stairs::DownEast));
	}
	return output;
}

bool Room::AnyRoomVetoes(const std::vector<Room> &roomVec) const
{
	for (const Room &room : roomVec)
	{
		if (room.VetoesRoom(*this))
		{
			return true;
		}
	}
	return false;
}

void Room::AddToMap(Map &map) const
{
	// Room positions are all in global space.

	// todo could probably make this better polymorphic design
	if (m_RoomType == RoomType::Stairs)
	{
		map.add_stairs(StairsLocalEnd(), m_StairsDirection);
		return;
	}

	map.fill_box(m_Box, Terrain::Open);

	// TODO doors
/*	if (IsCorridor() && CorridorLength() != 2 && !OneIn(3))
	{
		map.set_terrain(m_Box.min, Terrain::Door);
		map.set_terrain(m_Box.inner_max(), MapTerrain::Door);
	}*/
}
