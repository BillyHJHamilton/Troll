#include "Room.h"

#include "Debug.h"
#include "Math.h"
#include "Random.h"
#include "Serialize.h"

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

void Room::serialize(ISerializer& s)
{
	s.srz_box2(m_Box);
	s.srz_value(m_RoomType);
	s.srz_value(m_StairsDirection);
	s.srz_int(m_CorridorAxis);
	s.srz_int(m_Region);
	s.srz_vector<int>(m_Neighbours, "m_Neighbours");
}

Axis Room::CorridorAxis() const
{
	return m_CorridorAxis;
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

void Room::MarkCorridorAsMapConnector()
{
	if (!IsCorridor())
	{
		DebugBreak("You are only supposed to use this for corridors.");
	}

	m_RoomType = RoomType::IntermapCorridor;
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

int Room::GetNeighbourCount() const
{
	return Util::Size(m_Neighbours);
}

bool Room::IsNeighbourOf(int neighbour) const
{
	for (int i = 0; i < m_Neighbours.size(); ++i)
	{
		if (m_Neighbours[i] == neighbour)
		{
			return true;
		}
	}
	return false;
}

void Room::AddNeighbour(int neighbour)
{
	if (!IsNeighbourOf(neighbour))
	{
		m_Neighbours.push_back(neighbour);
	}
}

void Room::RemoveNeighbour(int neighbour)
{
	for (int i = 0; i < m_Neighbours.size(); ++i)
	{
		if (m_Neighbours[i] == neighbour)
		{
			m_Neighbours[i] = m_Neighbours.back();
			m_Neighbours.pop_back();
			return;  // found it!  don't search rest of list
		}
	}
}

void Room::RenumberNeighbour(int oldNeighbour, int newNeighbour)
{
	for (int i = 0; i < m_Neighbours.size(); ++i)
	{
		if (m_Neighbours[i] == oldNeighbour)
		{
			m_Neighbours[i] = newNeighbour;
			return;  // found it!  don't search rest of list
		}
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
		case RoomType::Corridor:
		case RoomType::IntermapCorridor:
			return JoinsToRoomAsCorridor(room);
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

Room::TempList Room::FindPossibleJoiningCorridors(Room const & other) const
{
	if (other.m_RoomType == RoomType::Stairs)
	{
		return Room::TempList();
	}

	return FindPossibleJoiningCorridorsToBox(other.GetBox());
}

Room::TempList Room::FindPossibleJoiningCorridorsToBox(Box2 other_box) const
{
	if (m_Box.intersects_or_adjacent(other_box))
	{
		return Room::TempList();
	}

	// Special case for stairs
	if (m_RoomType == RoomType::Stairs)
	{
		return FindPossibleJoiningCorridorsAsStairs(other_box);
	}

	// See in what manner we might join the rooms
	Axis overlapAxis;
	if (m_Box.overlaps_on_axis(other_box, AXIS_X))
	{
		overlapAxis = AXIS_X;
	}
	else if (m_Box.overlaps_on_axis(other_box, AXIS_Y))
	{
		overlapAxis = AXIS_Y;
	}
	else
	{
		return Room::TempList();
	}

	Axis corridorAxis = get_other_axis(overlapAxis);

	int minPos = std::max(m_Box.min[overlapAxis], other_box.min[overlapAxis]);
	int maxPos = std::min(m_Box.max()[overlapAxis], other_box.max()[overlapAxis]);

	Room::TempList output;
	output.reserve(maxPos + 1 - minPos);

	for (int pos = minPos; pos <= maxPos; ++pos)
	{
		output.push_back(FindPossibleJoiningCorridorCommon(other_box, corridorAxis, pos));
	}

	return output;
}

Room Room::FindPossibleJoiningCorridorCommon(Box2 other_box, Axis corridorAxis, int posOnOtherAxis) const
{
	Axis overlapAxis = get_other_axis(corridorAxis);

	// Make this a helper function
	if (other_box.min[corridorAxis] > m_Box.min[corridorAxis])
	{
		// It's on the positive side
		Box2 corridorBox;
		corridorBox.min[corridorAxis] = m_Box.max()[corridorAxis];
		corridorBox.size[corridorAxis] = other_box.min[corridorAxis] - corridorBox.min[corridorAxis];
		corridorBox.min[overlapAxis] = posOnOtherAxis;
		corridorBox.size[overlapAxis] = 1;
		return MakeCorridor(corridorBox, corridorAxis);
	}
	else
	{
		// It's on the negative side
		Box2 corridorBox;
		corridorBox.min[corridorAxis] = other_box.max()[corridorAxis];
		corridorBox.size[corridorAxis] = m_Box.min[corridorAxis] - corridorBox.min[corridorAxis];
		corridorBox.min[overlapAxis] = posOnOtherAxis;
		corridorBox.size[overlapAxis] = 1;
		return MakeCorridor(corridorBox, corridorAxis);
	}
}

Room::TempList Room::FindPossibleJoiningCorridorsAsStairs(Box2 other_box) const
{
	Room::TempList output;

	Axis corridorAxis = StairsAxis(m_StairsDirection);
	Axis overlapAxis = get_other_axis(corridorAxis);
	if (!m_Box.overlaps_on_axis(other_box, overlapAxis))
	{
		return output;
	}

	// They overlap, but is it on the correct side?
	int requiredSign = Stairs::joining_vector(m_StairsDirection)[corridorAxis];
	int roomDiff = other_box.min[corridorAxis] - m_Box.min[corridorAxis];
	if (!Math::SameSign(requiredSign, roomDiff))
	{
		return output;
	}

	// Find the one possible corridor and return it.
	int pos = StairsLocalEnd()[overlapAxis];
	output.push_back(FindPossibleJoiningCorridorCommon(other_box, corridorAxis, pos));
	return output;
}

Vec2 Room::AsStairsSuggestRandAdjoiningPositionForRoom(Vec2 roomSize) const
{
	if (m_RoomType != RoomType::Stairs)
	{
		DebugBreak("You are only supposed to use this for stairs.");
		return Vec2{0,0};
	}

	Axis axis = StairsAxis(m_StairsDirection);
	int dir = Stairs::joining_vector(m_StairsDirection)[axis];

	return SuggestRandAdjoiningPositionForRoomCommon(roomSize, StairsLocalEnd(), axis, dir);
}

Vec2 Room::AsCorridorSuggestRandAdjoiningPositionForRoom(Vec2 roomSize, Vec2 joinEnd) const
{
	if (!IsCorridor())
	{
		DebugBreak("You are only supposed to use this for corridors.");
		return Vec2{0,0};
	}

	Axis axis = CorridorAxis();

	int dir;
	if (joinEnd == m_Box.min)
	{
		dir = -1;
	}
	else if (joinEnd == m_Box.inner_max())
	{
		dir = 1;
	}
	else
	{
		DebugBreak("Join end does not correspond to either end of corridor.");
		return Vec2{0,0};
	}

	return SuggestRandAdjoiningPositionForRoomCommon(roomSize, joinEnd, axis, dir);
}

Vec2 Room::SuggestRandAdjoiningPositionForRoomCommon(Vec2 roomSize, Vec2 joinEnd, Axis axis, int dir) const
{
	Vec2 joinPosInRoom = joinEnd;
	joinPosInRoom[axis] += dir;

	Vec2 output;

	// Find the near side and far side of the room, and return the min of those
	// since that is where the origin-corner will be.
	int nearSide = joinPosInRoom[axis];
	int farSide = nearSide + (roomSize[axis]-1) * dir;
	output[axis] = std::min(nearSide, farSide);

	// Pick a random place side to side
	Axis slidingAxis = get_other_axis(axis);
	int minPos = joinPosInRoom[slidingAxis] - roomSize[slidingAxis] + 1;
	int maxPos = joinPosInRoom[slidingAxis];
	output[slidingAxis] = Random::in_range(minPos, maxPos);

	return output;
}

Room::TempList Room::FindPossibleJoiningStairs(bool goingUp) const
{
	Room::TempList output;

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

