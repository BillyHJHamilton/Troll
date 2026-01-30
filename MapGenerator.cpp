#include "MapGenerator.h"

#include "Debug.h"
#include "Map.h"
#include "Math.h"
#include "Random.h"
#include "Terrain.h"
#include "VectorUtil.h"

int constexpr c_MinRoomDimension = 3;
int constexpr c_MaxRoomDimension = 7;
int constexpr c_MinRoomArea = 3*3;
int constexpr c_MaxRoomArea = 6*6;

int constexpr c_MinNumRooms = 8;
int constexpr c_MaxNumRooms = 20;

int constexpr c_MinStairsProximity = 4;
int constexpr c_MinFacingStairsProximity = 3 + c_MinRoomDimension;

MapGenerator::MapGenerator(Map& owner)
	: m_Map(owner)
{
}

/*void MapGenerator::GetUpStairsFromDownStairs(MapGenerator const &levelAbove)
{
	for (Room const &room : levelAbove.m_RoomVec)
	{
		if (room.IsStairs() && !room.StairsGoUp(room.GetStairsDirection()))
		{
			m_UpStairs.push_back(StairsEntry(room.StairsRemoteEnd(),
				Room::StairsCorrespondingDirection(room.GetStairsDirection())));
		}
	}
}*/

void MapGenerator::Generate()
{
	m_RoomVec.clear();

	PlaceUpStairs();

	PlaceRooms();
	assert(Util::Size(m_RoomVec) > 0);

	AddJoiningCorridors();

	RemoveDisconnectedRooms();

	//AddDownStairs();

	AddExtraCorridors();

	m_Map.fill(Terrain::Wall);
	for (Room const & room : m_RoomVec)
	{
		assert(m_Map.contains(room.GetBox()));
		room.AddToMap(m_Map);
	}
}

//void MapGenerator::Print()
//{
//	m_Map.PrintMap();
//}

int MapGenerator::FindRoomAtPos(Vec2 pos)
{
	for (int i = 0; i < Util::Size(m_RoomVec); i++)
	{
		if (m_RoomVec[i].GetBox().contains(pos))
		{
			return i;
		}
	}
	return -1;
}

// Map Gen Helpers

void MapGenerator::PlaceRooms()
{
	// Other rooms
	int numRooms = Random::in_range(c_MinNumRooms,c_MaxNumRooms);

	int constexpr c_MaxAttempts = 1000;
	int attempts = 0;
	int numPlaced = 0;

	while (numPlaced < numRooms && attempts < c_MaxAttempts)
	{
		++ attempts;

		Box2 newRoomBox;
		newRoomBox.size = RandRoomSize();
		newRoomBox.min = RandRoomPos(newRoomBox.size);
		Room newRoom = Room::MakeChamber(newRoomBox);

		if (IsValidRoom(newRoom))
		{
			++numPlaced;
			m_RoomVec.push_back(newRoom);
		}
	}

	std::cout << "Placed " << numPlaced << " rooms; attempted "
		<< attempts << "; intended " << numRooms << "." << std::endl;
}

void MapGenerator::AddJoiningCorridors()
{
	// Joining rooms with corridors
	m_JoinedRooms.clear();
	m_JoinedRooms.reserve(Util::Size(m_RoomVec) * 2); // guess, not exact

	// To start with, any room with an up stairs is joined.
	for (Stairs::Pair &entry : m_UpStairs)
	{
		int r = FindRoomAtPos(entry.first);
		if (r != -1)
		{
			m_JoinedRooms.push_back(r);
		}
	}

	assert(Util::Size(m_JoinedRooms) > 0);

	int constexpr maxPasses = 4;
	int passes = 0;
	do
	{
		int numAddedThisPass = 0;

		// Try to join up all the rooms.
		for (int r0 = 0; r0 < m_RoomVec.size(); r0++)
		{
			// Always start from a joined room
			if (!Util::Contains(m_JoinedRooms, r0))
			{
				continue;
			}

			for (int r1 = 0; r1 < Util::Size(m_RoomVec); r1++)
			{
				// Don't join to self
				if (r0 == r1)
				{
					continue;
				}

				// Always go to an unjoined room.
				if (Util::Contains(m_JoinedRooms, r1))
				{
					continue;
				}

				// If it joins up by good fortune, mark it joined.
				if (m_RoomVec[r0].JoinsToRoom(m_RoomVec[r1])
					|| m_RoomVec[r0].JoinsToRoom(m_RoomVec[r1]))
				{
					m_JoinedRooms.push_back(r1);
					continue;
				}

				std::vector<Room> possibleCorridors =
					m_RoomVec[r0].FindPossibleJoiningCorridors(m_RoomVec[r1]);

				RemoveInavlidRoomsFromOptions(possibleCorridors);

				if (Util::Size(possibleCorridors) > 0)
				{
					++ numAddedThisPass;

					m_RoomVec.push_back(Random::from_vector(possibleCorridors));
					m_JoinedRooms.push_back(Util::Size(m_RoomVec)-1);
					m_JoinedRooms.push_back(r1);
				}
			}
		}

		std::cout << "Pass " << passes << " - Added " << numAddedThisPass
			<< " joining corridors." << std::endl;

		++ passes;
	}
	while (passes < maxPasses
		&& m_JoinedRooms.size() < m_RoomVec.size());
}

void MapGenerator::RemoveDisconnectedRooms()
{
	int deletedRooms = 0;

	// Traverse backwards so each index will still be correct when we reach it.
	for (int i = Util::Size(m_RoomVec) - 1; i >= 0; --i)
	{
		if (!Util::Contains(m_JoinedRooms, i))
		{
			++ deletedRooms;
			m_RoomVec[i] = m_RoomVec.back();
			m_RoomVec.pop_back();
		}
	}

	// The indices are all mixed up now, so delete the old array of indices.
	// Its work is done.
	m_JoinedRooms.clear();

	std::cout << "Deleted " << deletedRooms << " disconnected rooms."
		<< std::endl;
}

void MapGenerator::AddDownStairs()
{
	int constexpr c_NumDownStairs = 3;
	int constexpr c_MaxAttempts = 1000;

	int numAdded = 0;
	int attempts = 0;

	while(attempts < c_MaxAttempts && numAdded < c_NumDownStairs)
	{
		++attempts;

		int const r = Random::index(m_RoomVec);
		std::vector<Room> possibleStairs =
			m_RoomVec[r].FindPossibleJoiningDownStairs();

		RemoveInavlidRoomsFromOptions(possibleStairs);
		RemoveBadlyPlacedStairsFromOptions(possibleStairs);

		if (Util::Size(possibleStairs) > 0)
		{
			m_RoomVec.push_back(Random::from_vector(possibleStairs));
			++numAdded;
		}
	}

	std::cout << "Placed " << numAdded << " down stairs in "
		<< attempts << " attempts." << std::endl;
}

void MapGenerator::AddExtraCorridors()
{
	int numAdded = 0;
	
	for (int r0 = 0; r0 < Util::Size(m_RoomVec); r0++)
	{
		// Don't add extra connections to corridors.
		if (m_RoomVec[r0].IsCorridor())
		{
			continue;
		}

		for (int r1 = r0+1; r1 < Util::Size(m_RoomVec); r1++)
		{
			// Don't join to self
			if (r0 == r1)
			{
				continue;
			}

			// Don't add extra connections to corridors.
			if (m_RoomVec[r1].IsCorridor())
			{
				continue;
			}

			// Only a chance of adding extra corridors.
			if (!Random::one_in(3))
			{
				continue;
			}

			// Don't add another one the same.
			// Sorry for this awkward extra loop. :(
			bool alreadyJoined = false;
			for (const Room& room : m_RoomVec)
			{
				if (room.IsCorridor()
					&& room.JoinsToRoom(m_RoomVec[r0])
					&& room.JoinsToRoom(m_RoomVec[r1]))
				{
					alreadyJoined = true;
					break;
				}
			}
			if (alreadyJoined)
			{
				continue;
			}

			std::vector<Room> possibleCorridors =
				m_RoomVec[r0].FindPossibleJoiningCorridors(m_RoomVec[r1]);

			// Reduce probability of awkawardly sized extra corridors.
			if (Util::Size(possibleCorridors) > 0)
			{
				int length = possibleCorridors[0].CorridorLength();
				if ((length < 3 || length > 7)
					&& !Random::one_in(3))
				{
					continue;
				}
			}

			RemoveInavlidRoomsFromOptions(possibleCorridors);

			if (Util::Size(possibleCorridors) > 0)
			{
				++ numAdded;
				m_RoomVec.push_back(Random::from_vector(possibleCorridors));
			}
		}
	}

	std::cout << "Added " << numAdded << " extra corridors." << std::endl;
}

// Map Gen Helper Helpers

void MapGenerator::PlaceUpStairs()
{
	if (Util::Size(m_UpStairs) == 0)
	{
		// Add an up stairs if there isn't one (for first level).
		bool const isUp = true;
		m_UpStairs.push_back( RandStairsPos(isUp) );
	}

	int stairsAdded = 0;
	int otherRoomsAdded = 0;
	int corridorsAdded = 0;

	std::vector<int> addedStairs;

	for (Stairs::Pair const &entry : m_UpStairs)
	{
		Room stairsRoom = Room::MakeStairs(entry.first, entry.second);
		assert(IsValidRoom(stairsRoom));
		addedStairs.push_back(Util::Size(m_RoomVec));
		m_RoomVec.push_back(stairsRoom);

		++stairsAdded;
	}

	// Furthermore, give each of those stairs a landing room, if we can.
	for (int i : addedStairs)
	{
		Room const &stairsRoom = m_RoomVec[i];

		int constexpr c_MaxAttempts = 100;
		int attempts = 0;
		while (attempts < c_MaxAttempts)
		{
			Vec2 roomSize = RandRoomSize();
			Vec2 roomPos =
				stairsRoom.SuggestRandAdjoiningPositionForRoom(roomSize);
			Room adjoiningRoom = Room::MakeChamber(
				Box2(roomPos.x, roomPos.y, roomSize.x, roomSize.y));
			if (IsValidRoom(adjoiningRoom))
			{
				++otherRoomsAdded;
		
				m_RoomVec.push_back(adjoiningRoom);
				break;
			}

			++ attempts;
		}

		if (attempts == c_MaxAttempts)
		{
			// Failed to place a landing room.
			// Maybe we can do a corridor?
			bool success = false;
			for (int r1 = 0; r1 < Util::Size(m_RoomVec); ++r1)
			{
				if (m_RoomVec[r1].IsStairs())
				{
					continue;
				}

				std::vector<Room> options =
					stairsRoom.FindPossibleJoiningCorridors(m_RoomVec[r1]);
				
				RemoveInavlidRoomsFromOptions(options);

				if (options.size() > 0)
				{
					m_RoomVec.push_back(options[0]);
					++ corridorsAdded;
					break;
				}
			}
		}
	}

	std::cout << "Added " << stairsAdded << " up stairs, "
		<< otherRoomsAdded << " adjoining rooms, and "
		<< corridorsAdded << " corridors." << std::endl;
}

Vec2 MapGenerator::RandRoomSize()
{
	Vec2 size{
		Random::in_range(c_MinRoomDimension, c_MaxRoomDimension),
		Random::in_range(c_MinRoomDimension, c_MaxRoomDimension)
	};

	while (size.x * size.y > c_MaxRoomArea)
	{
		if (Random::coinflip() && size.x > c_MinRoomDimension)
		{
			-- size.x;
		}
		else if (size.y > c_MinRoomDimension)
		{
			-- size.y;
		}
		else
		{
			std::cerr << "Minimum room is too large: x = " << size.x << "; y = " << size.y << "; max area = " << c_MaxRoomArea << std::endl;
			break;
		}
	}

	while (size.x * size.y < c_MinRoomArea)
	{
		if (Random::coinflip() && size.x < c_MaxRoomDimension)
		{
			++ size.x;
		}
		else if (size.y < c_MaxRoomDimension)
		{
			++ size.y;
		}
		else
		{
			std::cerr << "Maximum room is too small: x = " << size.x << "; y = " << size.y << "; min area = " << c_MinRoomArea << std::endl;
			break;
		}
	}

	return size;
}

Vec2 MapGenerator::RandRoomPos(Vec2 size)
{
	Box2 valid_area = m_Map.get_box();
	valid_area.size -= {size};
	return Random::in_box(valid_area);
}

Stairs::Pair MapGenerator::RandStairsPos(bool isUp)
{
	Stairs::Direction dir = Stairs::DownEast;
	if (isUp)
	{
		dir = Stairs::random_up_direction();
	}
	else
	{
		dir = Stairs::random_down_direction();
	}
	Axis const stairsAxis = Room::StairsAxis(dir);
	Axis const otherAxis = get_other_axis(stairsAxis);

	Box2 map_box = m_Map.get_box();

	Vec2 stairsPos;
	stairsPos[otherAxis] = Random::in_range(
		map_box.min[otherAxis] + 2,
		map_box.inner_max(otherAxis) - 2);

	// Make sure not to put the stairs too close to the edge of the map
	// such that it's impossible to fit a nice big room in there.
	if (Stairs::joining_vector(dir)[stairsAxis] > 0)
	{
		stairsPos[stairsAxis] = Random::in_range(
			map_box.min[stairsAxis] + 2,
			map_box.inner_max(stairsAxis) - 2 - c_MaxRoomDimension);
	}
	else
	{
		stairsPos[stairsAxis] = Random::in_range(
			map_box.min[stairsAxis] + 2 + c_MaxRoomDimension,
			map_box.inner_max(stairsAxis) - 2);
	}

	return Stairs::Pair(stairsPos, dir);
}

bool MapGenerator::IsValidRoom(Room const &room)
{
	return m_Map.contains(room.GetBox())
		&& !room.AnyRoomVetoes(m_RoomVec);
}

void MapGenerator::RemoveInavlidRoomsFromOptions(std::vector<Room> &options)
{
	// Remove invalid corridors from the list
	options.erase(std::remove_if(options.begin(), options.end(),
			[this](const Room &corridor)
			{
				return !IsValidRoom(corridor);
			}
		), options.cend());
}

void MapGenerator::RemoveBadlyPlacedStairsFromOptions(std::vector<Room> &options)
{
	options.erase(std::remove_if(options.begin(), options.end(),
			[this](const Room &new_stairs)
			{
				return IsBadlyPlacedStairs(new_stairs);
			}
		), options.cend());
}

bool MapGenerator::IsBadlyPlacedStairs(Room const& new_stairs)
{
	// It's not good if it's going to run into the edge of the map.
	Stairs::Direction dir = new_stairs.GetStairsDirection();
	Vec2 stairsStart = new_stairs.StairsLocalEnd();
	Vec2 stairsVec = Stairs::relative_move(dir).xy();
	Vec2 stairsGoTowards = stairsStart + (c_MaxRoomDimension + 2)*stairsVec;
	if (!m_Map.contains(stairsGoTowards))
	{
		return true;
	}

	// It's not good if it's too close to another stairs.
	for (Room const &room : m_RoomVec)
	{
		if (room.IsStairs())
		{
			if (AreStairsProblematic(new_stairs, room))
			{
				return true;
			}
		}
	}

	return false;
}

bool MapGenerator::AreStairsProblematic(Room const& new_stairs, Room const& other_stairs)
{
	Vec2 p0 = new_stairs.StairsLocalEnd();
	Vec2 p1 = other_stairs.StairsLocalEnd();

	if(within_range(p0, p1, c_MinStairsProximity))
	{
		return true;
	}

	// It's especially bad if the stairs face each
	// other without space for a room in between.
	Axis a0 = Room::StairsAxis(new_stairs.GetStairsDirection());
	Axis a1 = Room::StairsAxis(other_stairs.GetStairsDirection());

	if (a0 == a1)
	{
		// They're on the same axis.
		// Are they far apart on the other axis?
		// If so, there's no problem.
		Axis otherAxis = get_other_axis(a0);
		int sideDiff = abs(p0[otherAxis] - p1[otherAxis]);
		if (sideDiff < c_MinRoomDimension + 2)
		{
			// They're close to lined up.
			// But maybe they are facing away from each other?
			Vec2 v0 = Stairs::relative_move(new_stairs.GetStairsDirection()).xy();
			Vec2 v1 = Stairs::relative_move(other_stairs.GetStairsDirection()).xy();
			if (!Math::SameSign(v0[a0], p0[a0] - p1[a0]) ||
				!Math::SameSign(v1[a0], p1[a0] - p0[a0]))
			{
				// They are not facing away from each other.
				// So check if the distance is adequate.
				int facingDist = abs(p0[a0] - p1[a0]);
				if (facingDist < c_MinFacingStairsProximity)
				{
					// Extremely Problematic!
					return true;
				}
			}
		}
	}

	// guess it's ok
	return false;
}
