#include "MapGenerator.h"

#include "Debug.h"
#include "Map.h"
#include "Math.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Serialize.h"
#include "Terrain.h"
#include "VectorUtil.h"

MapGenerator::MapGenerator(Map& owner)
	: m_Map(owner)
{
}

void MapGenerator::Parameters::Serialize(ISerializer& s)
{
	s.srz_int(MapBorder);

	s.srz_int(MinRoomDimension);
	s.srz_int(MaxRoomDimension);
	s.srz_int(MinRoomArea);
	s.srz_int(MaxRoomArea);

	s.srz_int(MinNumRooms);
	s.srz_int(MaxNumRooms);

	s.srz_int(MinStairsProximity);
	s.srz_int(MinFacingStairsProximity);

	s.srz_int(UpStairsToAdd);
	s.srz_int(DownStairsToAdd);
}

void MapGenerator::Serialize(ISerializer& s)
{
	srz_vector_size(s, m_SeedRooms, "m_SeedRooms");
	for (Room& r : m_SeedRooms)
	{
		r.Serialize(s);
	}

	srz_vector_size(s, m_RoomVec, "m_RoomVec");
	for (Room& r : m_RoomVec)
	{
		r.Serialize(s);
	}

	// Shouldn't need to save this.
	assert(m_JoinedRooms.empty());

	srz_vector(s, m_FailedStairs, "m_FailedStairs");

	// Can't serialize this.  Should be setup during construction.
	//Map& m_Map;

	m_Param.Serialize(s);

	// Presumably yes...
	s.srz_bool(m_HasGenerated);
}

void MapGenerator::AddConnectingStairsAsSeedRooms(Map const& other)
{
	int my_z = m_Map.get_z();

	bool add_up;
	if (other.get_z() == my_z + 1)
	{
		// Other level is above this one.
		// We will add up stairs corresponding to its down stairs;
		add_up = true;
	}
	else if (other.get_z() == my_z - 1)
	{
		// Other level is below this one.
		// We will add down stairs corresponding to its up stairs.
		add_up = false;
	}
	else
	{
		// Level is not one z away.  We can do nothing.
		DebugBreak("Trying to connect stairs with z out of range.");
		return;
	}

	for (const Stairs::Pair& pair : other.get_stairs_map())
	{
		Vec2 const start_pos = pair.first;
		Stairs::Direction const dir = pair.second;
		if (add_up != Stairs::is_up(dir))
		{
			Vec2 this_end = start_pos + Stairs::relative_move(dir).xy();
			if (m_Map.contains(this_end))
			{
				Stairs::Direction const reverse = Stairs::reverse(dir);
				Room new_stairs = Room::MakeStairs(this_end, reverse);
				if (IsValidRoom(new_stairs, /*check border*/ true))
				{
					m_SeedRooms.push_back(new_stairs);
				}
			}
		}
	}
}

void MapGenerator::Generate()
{
	if (c_ShowMapDebug)
	{
		std::cout << "\nGenerating level.\n";
	}

	PerfTimer perf0("map generate");

	PlaceSeedRooms();

	PlaceRooms();
	assert(Util::Size(m_RoomVec) > 0);

	AddJoiningCorridors();

	RemoveDisconnectedRooms();

	AddExtraStairs(/*goingUp*/ true, m_Param.UpStairsToAdd);
	AddExtraStairs(/*goingUp*/ false, m_Param.DownStairsToAdd);

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

void MapGenerator::PlaceSeedRooms()
{
	m_FailedStairs.clear();

	if (Util::Size(m_SeedRooms) == 0)
	{
		// Add a random chamber as our seed room.
		Room newRoom = MakeRandomChamber();
		m_SeedRooms.push_back(newRoom);
	}

	for (Room const& room : m_SeedRooms)
	{
		assert(IsValidRoom(room, /*check border*/ false));
	}

	// Copy the seeds into the main room vector.
	// We also keep the original seeds in case we want to regenerate later.
	m_RoomVec = m_SeedRooms;
	int const totalSeedRooms = Util::Size(m_RoomVec);

	int seedStairs = 0;
	int seedCorridors = 0;
	int seedChambers = 0;
	int failedStairs = 0;
	int landingChambers = 0;
	int landingCorridors = 0;

	for (int r = totalSeedRooms - 1; r >= 0; --r)
	{	
		if (m_RoomVec[r].IsCorridor())
		{
			++seedCorridors;
		}
		else if (m_RoomVec[r].IsChamber())
		{
			++seedChambers;
		}
		else if (m_RoomVec[r].IsStairs())
		{
			// Give each seed staircase a landing room, if we can.
			bool const success = TryAddLanding(m_RoomVec[r], landingChambers, landingCorridors);

			// If we failed to place a landing room, the staircase will end up disconnected.
			// Remove it and note it in the failed rooms list.
			if (success)
			{
				++seedStairs;
			}
			else
			{
				Stairs::Pair data{
					m_RoomVec[r].StairsLocalEnd(),
					m_RoomVec[r].GetStairsDirection()
				};
				m_FailedStairs.push_back(data);
				Util::RemoveSwap(m_RoomVec, r);
				++failedStairs;
			}
		}
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format(
			"Seed Rooms: {} stairs ({} failed), {} chambers, and {} corridors.\n"
			"Landings: added {} chambers, {} corridors.\n",
			seedStairs, failedStairs, seedChambers, seedCorridors,
			landingChambers, landingCorridors);
	}
}

void MapGenerator::PlaceRooms()
{
	// Other rooms
	int numRooms = Random::in_range(m_Param.MinNumRooms, m_Param.MaxNumRooms);

	int constexpr c_MaxAttempts = 1000;
	int attempts = 0;
	int numPlaced = 0;

	while (numPlaced < numRooms && attempts < c_MaxAttempts)
	{
		++ attempts;

		Room newRoom = MakeRandomChamber();

		if (IsValidRoom(newRoom, /*check border*/ true))
		{
			++numPlaced;
			m_RoomVec.push_back(newRoom);
		}
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Placed {} rooms; attempted {}; intended {}.\n",
			numPlaced, attempts, numRooms);
	}
}

void MapGenerator::AddJoiningCorridors()
{
	// Joining rooms with corridors
	m_JoinedRooms.clear();
	m_JoinedRooms.reserve(Util::Size(m_RoomVec) * 2); // guess, not exact

	// Seed rooms are assumed to be connected into the rest of the world.
	// They basically represent known entrances to the level.
	// If necessary, we could add required "exit" rooms that need to be joined in.
	// We may need a more sophisticated approach to connectivity in the future.
	for (int i = 0; i < Util::Size(m_SeedRooms); ++i)
	{
		// Seed room should still be at the start of the list.
		m_JoinedRooms.push_back(i);
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

		if (c_ShowMapDebug)
		{
			std::cout << std::format("Pass {} - Added {} joining corridors.\n",
				passes, numAddedThisPass);
		}

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
			Util::RemoveSwap(m_RoomVec, i);
		}
	}

	// The indices are all mixed up now, so delete the old array of indices.
	// Its work is done.
	m_JoinedRooms.clear();

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Deleted disconnected rooms.\n",
			deletedRooms);
	}
}

void MapGenerator::AddExtraStairs(bool goingUp, int stairsToAdd)
{
	int constexpr c_MaxAttempts = 1000;

	int numAdded = 0;
	int attempts = 0;

	while(attempts < c_MaxAttempts && numAdded < stairsToAdd)
	{
		++attempts;

		int const r = Random::index(m_RoomVec);
		std::vector<Room> possibleStairs =
			m_RoomVec[r].FindPossibleJoiningStairs(goingUp);

		RemoveInavlidRoomsFromOptions(possibleStairs);
		RemoveBadlyPlacedStairsFromOptions(possibleStairs);

		if (Util::Size(possibleStairs) > 0)
		{
			m_RoomVec.push_back(Random::from_vector(possibleStairs));
			++numAdded;
		}
	}

	if (c_ShowMapDebug)
	{
		std::cout << std::format("Placed {}/{} {} stairs in {} attempts.\n",
			numAdded, stairsToAdd, (goingUp ? " up" : " down"), attempts);
	}
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
	
	if (c_ShowMapDebug)
	{
		std::cout << "Added " << numAdded << " extra corridors." << std::endl;
	}
}

// Map Gen Helper Helpers

Vec2 MapGenerator::RandRoomSize()
{
	Vec2 size{
		Random::in_range(m_Param.MinRoomDimension, m_Param.MaxRoomDimension),
		Random::in_range(m_Param.MinRoomDimension, m_Param.MaxRoomDimension)
	};

	while (size.x * size.y > m_Param.MaxRoomArea)
	{
		if (Random::coinflip() && size.x > m_Param.MinRoomDimension)
		{
			-- size.x;
		}
		else if (size.y > m_Param.MinRoomDimension)
		{
			-- size.y;
		}
		else
		{
			std::cerr << "Minimum room is too large: x = " << size.x << "; y = " << size.y
				<< "; max area = " << m_Param.MaxRoomArea << std::endl;
			break;
		}
	}

	while (size.x * size.y < m_Param.MinRoomArea)
	{
		if (Random::coinflip() && size.x < m_Param.MaxRoomDimension)
		{
			++ size.x;
		}
		else if (size.y < m_Param.MaxRoomDimension)
		{
			++ size.y;
		}
		else
		{
			std::cerr << "Maximum room is too small: x = " << size.x << "; y = " << size.y << "; min area = " << m_Param.MinRoomArea << std::endl;
			break;
		}
	}

	return size;
}

Vec2 MapGenerator::RandRoomPos(Vec2 roomSize)
{
	Box2 valid_area = m_Map.get_box();
	valid_area.min += {m_Param.MapBorder, m_Param.MapBorder};
	valid_area.size -= {2*m_Param.MapBorder, 2*m_Param.MapBorder};
	valid_area.size -= {roomSize};
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
	int const smallBorder = 1 + m_Param.MapBorder;
	int const largeBorder = smallBorder + m_Param.MaxRoomDimension;

	Vec2 stairsPos;
	stairsPos[otherAxis] = Random::in_range(
		map_box.min[otherAxis] + smallBorder,
		map_box.inner_max(otherAxis) - smallBorder);

	// Make sure not to put the stairs too close to the edge of the map
	// such that it's impossible to fit a nice big room in there.
	if (Stairs::joining_vector(dir)[stairsAxis] > 0)
	{
		stairsPos[stairsAxis] = Random::in_range(
			map_box.min[stairsAxis] + smallBorder,
			map_box.inner_max(stairsAxis) - largeBorder);
	}
	else
	{
		stairsPos[stairsAxis] = Random::in_range(
			map_box.min[stairsAxis] + largeBorder,
			map_box.inner_max(stairsAxis) - smallBorder);
	}

	return Stairs::Pair(stairsPos, dir);
}

Room MapGenerator::MakeRandomChamber()
{
	Box2 newRoomBox;
	newRoomBox.size = RandRoomSize();
	newRoomBox.min = RandRoomPos(newRoomBox.size);
	return Room::MakeChamber(newRoomBox);
}

bool MapGenerator::IsValidRoom(Room const &room, bool checkBorder)
{
	Box2 boundingBox = m_Map.get_box();
	if (checkBorder)
	{
		boundingBox.min += Vec2{m_Param.MapBorder, m_Param.MapBorder};
		boundingBox.size -= Vec2{2*m_Param.MapBorder, 2*m_Param.MapBorder};
	}

	return boundingBox.contains(room.GetBox())
		&& !room.AnyRoomVetoes(m_RoomVec);
}

bool MapGenerator::TryAddLanding(Room const &stairsRoom, int& chambersAdded, int& corridorsAdded)
{
	int constexpr c_MaxAttempts = 100;
	for (int attempts = 0; attempts < c_MaxAttempts; ++attempts)
	{
		Vec2 roomSize = RandRoomSize();
		Vec2 roomPos =
			stairsRoom.SuggestRandAdjoiningPositionForRoom(roomSize);
		Room adjoiningRoom = Room::MakeChamber(
			Box2(roomPos.x, roomPos.y, roomSize.x, roomSize.y));
		if (IsValidRoom(adjoiningRoom, /*check border*/ true))
		{
			m_RoomVec.push_back(adjoiningRoom);
			++chambersAdded;
			return true;
		}
	}

	// Failed to place a landing room.
	// Maybe we can do a corridor?
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
			return true;
		}
	}

	return false;
}

void MapGenerator::RemoveInavlidRoomsFromOptions(std::vector<Room> &options)
{
	// Remove invalid corridors from the list
	options.erase(std::remove_if(options.begin(), options.end(),
			[this](const Room &corridor)
			{
				return !IsValidRoom(corridor, /*check border*/ true);
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
	Vec2 stairsGoTowards = stairsStart + (m_Param.MaxRoomDimension + 2)*stairsVec;
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

	if(within_range(p0, p1, m_Param.MinStairsProximity))
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
		if (sideDiff < m_Param.MinRoomDimension + 2)
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
				if (facingDist < m_Param.MinFacingStairsProximity)
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
