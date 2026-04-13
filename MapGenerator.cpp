#include "MapGenerator.h"

#include "Debug.h"
#include "Map.h"
#include "Math.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Serialize.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

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
}

void MapGenerator::Region::serialize(ISerializer& s)
{
	s.srz_int(parent);
	s.srz_vector(rooms, "rooms");
}

void MapGenerator::Serialize(ISerializer& s)
{
	s.srz_vector(m_RequestedConnections, "m_RequestedConnections");

	s.srz_vector_advanced(m_RoomVec, "m_RoomVec");

	// Shouldn't need to save this.
	assert(m_JoinedRooms.empty());

	s.srz_vector_advanced(m_RegionVec, "m_RegionVec");

	// Can't serialize this.  Should be setup during construction.
	//Map& m_Map;

	m_Param.Serialize(s);

	// Presumably yes...
	s.srz_bool(m_HasGenerated);
}

void MapGenerator::RequestConnection(int targetMapId, int numToAdd)
{
	m_RequestedConnections.push_back({targetMapId, numToAdd});
}

void MapGenerator::Generate()
{
	if (Debug::enabled(Debug::Map))
	{
		// name is not set yet, so can't print
		std::cout << "\nGenerating level.\n";
	}

	PerfTimer perf0("map generate");

	PlaceFirstRoomIfNeeded();

	// TODO We likely require a more sophisticated approach to inner connectivity in the future.
	MarkExistingRoomsJoined();

	PlaceRooms();
	assert(Util::Size(m_RoomVec) > 0);

	AddJoiningCorridors();

	RemoveDisconnectedRooms();

	for (RequestedConnection req : m_RequestedConnections)
	{
		// sorry, didn't want to #include World here, but seems necessary
		Map& other_map = World::edit().edit_map(req.target_level);

		if (other_map.get_z() == m_Map.get_z())
		{
			AddTunnelTo(other_map.get_generator(), req.num_to_add);
		}
		else
		{
			AddStairsTo(other_map.get_generator(), req.num_to_add);
		}
	}

	AddExtraCorridors();

	AssignRoomsToRegions();

	// TODO: Add secret corridors here

	//PrintAllRooms();

	AddAllToMap();
}

void MapGenerator::AddTunnelTo(MapGenerator& other, int numToAdd)
{
	if (other.m_Map.get_z() != m_Map.get_z())
	{
		DebugBreak("Cannot tunnel with different z.");
		return;
	}

	// Figure out what direction we are on.
	Box2 const my_box = m_Map.get_box();
	Box2 const other_box = other.m_Map.get_box();

	CompassDirection const adjacent_edge = my_box.adjacent_edge(other_box);
	if (adjacent_edge == c_CompassInvalid)
	{
		DebugBreak("No tunnel connection possible as maps are not adjacent.");
		return;		
	}

	Axis const join_axis = (adjacent_edge == c_CompassEast || adjacent_edge == c_CompassWest) ?
		AXIS_X : AXIS_Y;

	// Establish the area we are trying to link to.
	Box2 border_box = my_box.outer_border_box(adjacent_edge);
	border_box = border_box.intersection(other_box);

	// Now try to join with each room
	IntTempList rooms_to_try = Util::GetIndices(m_RoomVec);
	Random::shuffle_vector(rooms_to_try);
	Room::TempList possible_tunnels;

	int num_added = 0;
	while (num_added < numToAdd && !rooms_to_try.empty())
	{
		int const room_index = rooms_to_try.back();
		Room& room = m_RoomVec[room_index];
		rooms_to_try.pop_back();

		if (room.GetRoomType() == RoomType::Chamber)
		{
			possible_tunnels = room.FindPossibleJoiningCorridorsToBox(border_box);

			bool added = false;
			while (!added && !possible_tunnels.empty())
			{
				int const t = Random::index(possible_tunnels);
				Room& tunnel = possible_tunnels[t];

				if (IsValidRoom(tunnel, /*check_border*/ false))
				{
					// But will the other side take it?
					Axis sliding_axis = get_other_axis(join_axis);
					Vec2 entry_point;
					entry_point[join_axis] = border_box.min[join_axis];
					entry_point[sliding_axis] = tunnel.GetBox().min[sliding_axis];

					bool const success = other.TryReceiveTunnel(entry_point, join_axis);

					if (success)
					{
						tunnel.MarkCorridorAsMapConnector();
						m_RoomVec.push_back(tunnel);
						added = true;
						++num_added;

						int const tunnel_index = Util::LastIndex(m_RoomVec);
						m_RoomVec[room_index].AddNeighbour(tunnel_index);
						m_RoomVec[tunnel_index].AddNeighbour(room_index);
					}
				}

				Util::RemoveSwap(possible_tunnels, t);
			}
		}
	}
	
	// Todo: We'd prefer a shorter tunnel, but I'm not sure of an efficient way to do that.

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Placed {}/{} tunnels.\n", num_added, numToAdd);
	}
}

void MapGenerator::AddStairsTo(MapGenerator& other, int numToAdd)
{
	int const my_z = m_Map.get_z();
	int const other_z = other.m_Map.get_z();

	if (std::abs(my_z - other_z) != 1)
	{
		DebugBreak("Invalid staircase range.");
		return;
	}
	bool const goingUp = other_z > my_z;

	Box2 const my_box = m_Map.get_box();
	Box2 const other_box = other.m_Map.get_box();

	if (!my_box.intersects(other_box))
	{
		DebugBreak("Maps do not overlap.");
		return;
	}

	int numAdded = 0;

	IntTempList roomIndices = Util::GetIndices(m_RoomVec);
	Random::shuffle_vector(roomIndices);

	Room::TempList possibleStairs;

	// Outer loop: Try every room until we place all our stairs.
	while(numAdded < numToAdd && !roomIndices.empty())
	{
		bool success = false;
		int const r = roomIndices.back();
		possibleStairs = m_RoomVec[r].FindPossibleJoiningStairs(goingUp);

		RemoveInvalidRoomsFromOptions(possibleStairs, /*check_border*/ true);
		RemoveBadlyPlacedStairsFromOptions(possibleStairs, other_box);

		if (Util::Size(possibleStairs) > 0)
		{
			// Inner loop: We've got some stairs that look OK from this side.
			// Now try them in random order until one looks OK from the receiving side.
			Random::shuffle_vector(possibleStairs);
			for (Room& newStairs : possibleStairs)
			{
				Stairs::Pair stairs_pair{
					newStairs.StairsLocalEnd(),
					newStairs.GetStairsDirection()
				};
				bool received = other.TryReceiveStairs(my_z, stairs_pair);
				if (received)
				{
					m_RoomVec.push_back(newStairs);
					++numAdded;

					int const stairs_index = Util::LastIndex(m_RoomVec);
					m_RoomVec[r].AddNeighbour(stairs_index);
					m_RoomVec[stairs_index].AddNeighbour(r);

					success = true;
					break; // We succeeded, so exit inner loop.
				}
			}
		}

		if (success)
		{
			// We could try this room again later.  Stick it somewhere in the list.
			std::swap(roomIndices.back(), roomIndices[Random::index(roomIndices)]);
		}
		else
		{
			// This room is tapped out.
			roomIndices.pop_back();
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Placed {}/{} {} stairs.\n",
			numAdded, numToAdd, (goingUp ? " up" : " down"));
	}

}

bool MapGenerator::TryReceiveTunnel(Vec2 entry_point, Axis corridor_axis)
{
	int dir;
	if (entry_point[corridor_axis] == m_Map.get_box().min[corridor_axis])
	{
		dir = 1;
	}
	else
	{
		assert(entry_point[corridor_axis] == m_Map.get_box().inner_max(corridor_axis));
		dir = -1;
	}

	// corridor could be, say, 1-3 units long.
	IntTempList lengths;
	Util::FillAscending(lengths, 3, 1);

	Random::shuffle_vector(lengths);
	for (int length : lengths)
	{
		Vec2 other_end = entry_point;
		other_end[corridor_axis] += dir*(length - 1);
		Box2 box = Box2::spanning(entry_point, other_end);
		Room corridor = Room::MakeCorridor(box, corridor_axis);
		corridor.MarkCorridorAsMapConnector();

		if (IsValidRoom(corridor, /*checkBorder*/ false))
		{
			m_RoomVec.push_back(corridor);

			// So far so good, but can we fit a landing?
			int const corridorRoomIndex = Util::LastIndex(m_RoomVec);
			bool const placed_room =
				TryAddAdjoiningRoomForCorridor(corridorRoomIndex, other_end);

			if (placed_room)
			{
				return true;
			}
			else
			{
				// Bad room, no twinkie.
				// Can try again at the next length.
				m_RoomVec.pop_back();
			}
		}
	}

	return false;
}

bool MapGenerator::TryReceiveStairs(int sender_z, Stairs::Pair stairs_pair)
{
	int my_z = m_Map.get_z();
	bool const from_above = sender_z > my_z;

	if (std::abs(sender_z - my_z) != 1)
	{
		DebugBreak("Trying to connect stairs with z out of range.");
		return false;
	}

	Vec2 const start_pos = stairs_pair.first;
	Stairs::Direction const start_dir = stairs_pair.second;

	if (from_above == Stairs::is_up(start_dir))
	{
		DebugBreak("Trying to connect stairs in wrong direction.");
		return false;
	}

	Vec2 this_end = start_pos + Stairs::relative_move(start_dir).xy();
	if (!m_Map.contains(this_end))
	{
		DebugBreak("Trying to connect stairs outside of map.");
		return false;
	}

	Stairs::Direction const local_dir = Stairs::reverse(start_dir);
	Room new_stairs = Room::MakeStairs(this_end, local_dir);

	if (IsValidRoom(new_stairs, /*check border*/ true))
	{
		m_RoomVec.push_back(new_stairs);

		// So far so good, but can we fit a landing?
		int const stairsRoomIndex = Util::LastIndex(m_RoomVec);
		bool const placed_landing = TryAddLandingRoom(stairsRoomIndex);

		if (placed_landing)
		{
			return true;
		}
		else
		{
			// Bad room, no twinkie
			m_RoomVec.pop_back();
			return false;
		}
	}
	else
	{
		return false;
	}
}

int MapGenerator::FindRoomAtPos(Vec2 pos) const
{
	for (int i = 0; i < Util::Size(m_RoomVec); ++i)
	{
		if (m_RoomVec[i].GetBox().contains(pos))
		{
			return i;
		}
	}
	return -1;
}

// Map Gen Helpers

void MapGenerator::PlaceFirstRoomIfNeeded()
{
	if (Util::Size(m_RoomVec) == 0)
	{
		// Add a random chamber as our seed room.
		Room newRoom = MakeRandomChamber();
		m_RoomVec.push_back(newRoom);
		// no connections to add

		// assume this is the start room for the player
		Vec2 room_centre = newRoom.GetBox().centre();
		m_Map.edit_suggestions().add_player_start(room_centre);
	}
}

void MapGenerator::MarkExistingRoomsJoined()
{
	m_JoinedRooms.clear();

	// Just a guess.
	m_JoinedRooms.reserve(Util::Size(m_RoomVec) + 30);
	Util::FillAscending(m_JoinedRooms, Util::Size(m_RoomVec), 0);
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
			// don't add connections
			//  -> chambers never connect to each other
			//  -> connections to corridors will be added with corridors
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Placed {} rooms; attempted {}; intended {}.\n",
			numPlaced, attempts, numRooms);
	}
}

// Joining rooms with corridors
void MapGenerator::AddJoiningCorridors()
{
	assert(Util::Size(m_JoinedRooms) > 0);

	int constexpr maxPasses = 4;
	int passes = 0;
	do
	{
		int numAddedThisPass = 0;

		// Try to join up all the rooms.
		for (int r0 = 0; r0 < m_RoomVec.size(); ++r0)
		{
			// Always start from a joined room
			if (!Util::Contains(m_JoinedRooms, r0))
			{
				continue;
			}

			for (int r1 = 0; r1 < Util::Size(m_RoomVec); ++r1)
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
				if (m_RoomVec[r0].JoinsToRoom(m_RoomVec[r1]))
				{
					m_JoinedRooms.push_back(r1);
					continue;
				}

				Room::TempList possibleCorridors =
					m_RoomVec[r0].FindPossibleJoiningCorridors(m_RoomVec[r1]);

				RemoveInvalidRoomsFromOptions(possibleCorridors, /*check_border*/ true);

				if (Util::Size(possibleCorridors) > 0)
				{
					++ numAddedThisPass;

					m_RoomVec.push_back(Random::from_vector(possibleCorridors));
					int const corridor_index = Util::LastIndex(m_RoomVec);
					m_JoinedRooms.push_back(corridor_index);
					m_JoinedRooms.push_back(r1);

					m_RoomVec[r0].AddNeighbour(corridor_index);
					m_RoomVec[r1].AddNeighbour(corridor_index);
					m_RoomVec[corridor_index].AddNeighbour(r0);
					m_RoomVec[corridor_index].AddNeighbour(r1);
				}
			}
		}

		if (Debug::enabled(Debug::Map))
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
			RemoveRoomFromAllNeighbourLists(i);
			int const oldIndex = Util::LastIndex(m_RoomVec);
			RenumberRoomInAllNeighbourLists(oldIndex, i);

			++ deletedRooms;
			Util::RemoveSwap(m_RoomVec, i);
		}
	}

	// The indices are all mixed up now, so delete the old array of indices.
	// Its work is done.
	m_JoinedRooms.clear();

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Deleted {} disconnected rooms.\n",
			deletedRooms);
	}
}

void MapGenerator::AddExtraCorridors()
{
	int numAdded = 0;
	
	for (int r0 = 0; r0 < Util::Size(m_RoomVec); ++r0)
	{
		// Don't add extra connections to corridors.
		if (m_RoomVec[r0].IsCorridor())
		{
			continue;
		}

		for (int r1 = r0+1; r1 < Util::Size(m_RoomVec); ++r1)
		{
			// Don't join to self
			if (r0 == r1)
			{
				continue;
			}

			// Only add extra connections to chambers.
			//  -> not to corridors or stairs
			if (!m_RoomVec[r1].IsChamber())
			{
				continue;
			}

			// Only a chance of adding extra corridors.
			if (!Random::one_in(3))
			{
				continue;
			}

			// Don't add another one the same.
			if(AreRoomsAlreadyConnected(r0, r1))
			{
				continue;
			}

			Room::TempList possibleCorridors =
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

			RemoveInvalidRoomsFromOptions(possibleCorridors, /*check_border*/ true);

			if (Util::Size(possibleCorridors) > 0)
			{
				++ numAdded;
				m_RoomVec.push_back(Random::from_vector(possibleCorridors));

				int const corridor_index = Util::LastIndex(m_RoomVec);
				m_RoomVec[r0].AddNeighbour(corridor_index);
				m_RoomVec[r1].AddNeighbour(corridor_index);
				m_RoomVec[corridor_index].AddNeighbour(r0);
				m_RoomVec[corridor_index].AddNeighbour(r1);
			}
		}
	}
	
	if (Debug::enabled(Debug::Map))
	{
		std::cout << "Added " << numAdded << " extra corridors." << std::endl;
	}
}

void MapGenerator::AssignRoomsToRegions()
{
	int constexpr c_NoRegion = -1;
	int constexpr c_MultipleRegions = -2;

	PerfTimer perf0("rooms to regions");

	// each new region has a room and a corridor (or several of each)
	m_RegionVec.reserve(Util::Size(m_RoomVec) / 2);

	Region main_region;
	main_region.parent = Room::c_MainRegion;  // could have a special value
	m_RegionVec.push_back(main_region);

	bool isChanged = true;
	while (isChanged)
	{
		isChanged = false;

		for (int r = 0; r < Util::Size(m_RoomVec); ++r)
		{
			if (m_RoomVec[r].IsInMainRegion() &&
			    !m_RoomVec[r].IsMapConnector())
			{
				int mainRegionConnectionCount = 0;
				int currentRegion = c_NoRegion;
				bool isOutsideConnection = false;
				for (int n = 0; n < m_RoomVec[r].GetNeighbourCount(); ++n)
				{
					int const other_index = m_RoomVec[r].GetNeighbours()[n];

					if (m_RoomVec[other_index].IsMapConnector())
					{
						isOutsideConnection = true;
						break;  // nothing connected to another map is a dead end
					}

					if (!m_RoomVec[other_index].IsInMainRegion())
					{
						// maybe add this room to existing region
						//  -> if 0 regions leads off this, it becomes a new region
						//  -> if 1 regions leads off this, join it
						//  -> if 2 regions lead off this, it becomes a third region
						if (currentRegion == c_NoRegion)
						{
							currentRegion = m_RoomVec[other_index].GetRegion();
						}
						else
						{
							currentRegion = c_MultipleRegions;
						}
					}
					else
					{
						++mainRegionConnectionCount;
					}
				}

				if (isOutsideConnection)
				{
					continue;  // nothing connected to another map is a dead end
				}

				if (mainRegionConnectionCount == 0)
				{
					DebugBreak("Room has no non-dead-end connections, but it wasn't removed");
				}
				else if (mainRegionConnectionCount == 1)
				{
					// this room is a dead end; add it to a region

					if (currentRegion > Room::c_MainRegion)
					{
						// add to the existing region it is connected to
						m_RoomVec[r].SetRegion(currentRegion);
						m_RegionVec[currentRegion].rooms.push_back(r);
					}
					else // no existing region, or connects to multiple regions
					{
						// start a new region
						int const newRegionIndex = Util::Size(m_RegionVec);

						Region region;
						region.parent = Room::c_MainRegion;
						region.rooms.push_back(r);
						m_RegionVec.push_back(region);

						m_RoomVec[r].SetRegion(newRegionIndex);
					}

					MakeRoomARegionParent(r);

					isChanged = true;
				}
			}
		}
	}

	// add all unassigned rooms to main region
	for (int r = 0; r < Util::Size(m_RoomVec); ++r)
	{
		if (m_RoomVec[r].IsInMainRegion())
		{
			m_RegionVec[Room::c_MainRegion].rooms.push_back(r);
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Divided map into {} regions.\n", Util::Size(m_RegionVec));
	}
}

void MapGenerator::AddAllToMap()
{
	// step 0: fill map with walls
	m_Map.fill(Terrain::Wall);

	// step 1: add chambers
	for (Room const & room : m_RoomVec)
	{
		assert(m_Map.contains(room.GetBox()));
		if (room.IsChamber())
		{
			AddChamberToMap(room);
		}
	}

	// step 2: add stairs
	for (Room const & room : m_RoomVec)
	{
		assert(m_Map.contains(room.GetBox()));
		if (room.IsStairs())
		{
			AddStairsToMap(room);
		}
	}

	// step 3: add corridors
	for (Room const & room : m_RoomVec)
	{
		assert(m_Map.contains(room.GetBox()));
		if (room.IsCorridor())
		{
			AddCorridorToMap(room);
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Added all rooms to map, made {} suggestions.\n",
			m_Map.read_suggestions().get_total_count());
	}
}

// Map Gen Helper Helpers

Vec2 MapGenerator::RandRoomSize() const
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

Vec2 MapGenerator::RandRoomPos(Vec2 roomSize) const
{
	Box2 valid_area = m_Map.get_box();
	valid_area.min += {m_Param.MapBorder, m_Param.MapBorder};
	valid_area.size -= {2*m_Param.MapBorder, 2*m_Param.MapBorder};
	valid_area.size -= {roomSize};
	return Random::in_box(valid_area);
}

Stairs::Pair MapGenerator::RandStairsPos(bool isUp) const
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

Room MapGenerator::MakeRandomChamber() const
{
	Box2 newRoomBox;
	newRoomBox.size = RandRoomSize();
	newRoomBox.min = RandRoomPos(newRoomBox.size);
	return Room::MakeChamber(newRoomBox);
}

bool MapGenerator::IsValidRoom(Room const &room, bool checkBorder) const
{
	Box2 boundingBox = checkBorder ?
		m_Map.get_box_minus_border(m_Param.MapBorder) :
		m_Map.get_box();

	return boundingBox.contains(room.GetBox())
		&& (!room.IsCorridor() || room.CorridorLength() <= m_Param.MaxCorridorLength)
		&& !room.AnyRoomVetoes(m_RoomVec);
}

bool MapGenerator::TryAddLandingRoom(int stairsRoomIndex)
{
	// must not pass in stairsRoom by reference
	//  -> this function adds to the vector, which could cause a re-allocate

	int constexpr c_MaxAttempts = 100;
	for (int attempts = 0; attempts < c_MaxAttempts; ++attempts)
	{
		Vec2 roomSize = RandRoomSize();
		Vec2 roomPos =
			m_RoomVec[stairsRoomIndex].AsStairsSuggestRandAdjoiningPositionForRoom(roomSize);
		Room adjoiningRoom = Room::MakeChamber(
			Box2(roomPos.x, roomPos.y, roomSize.x, roomSize.y));
		if (IsValidRoom(adjoiningRoom, /*check border*/ true))
		{
			int const adjoiningRoomIndex = Util::Size(m_RoomVec);
			m_RoomVec[stairsRoomIndex].AddNeighbour(adjoiningRoomIndex);
			adjoiningRoom.AddNeighbour(stairsRoomIndex);
			m_RoomVec.push_back(adjoiningRoom);
			return true;
		}
	}

	return false;
}

bool MapGenerator::TryAddAdjoiningRoomForCorridor(int corridorRoomIndex, Vec2 joinEnd)
{
	// must not pass in corridorRoom by reference
	//  -> this function adds to the vector, which could cause a re-allocate

	int constexpr c_MaxAttempts = 100;
	for (int attempts = 0; attempts < c_MaxAttempts; ++attempts)
	{
		Vec2 roomSize = RandRoomSize();
		Vec2 roomPos =
			m_RoomVec[corridorRoomIndex].AsCorridorSuggestRandAdjoiningPositionForRoom(roomSize, joinEnd);
		Room adjoiningRoom = Room::MakeChamber(
			Box2(roomPos.x, roomPos.y, roomSize.x, roomSize.y));
		if (IsValidRoom(adjoiningRoom, /*check border*/ true))
		{
			int const adjoiningRoomIndex = Util::Size(m_RoomVec);
			m_RoomVec[corridorRoomIndex].AddNeighbour(adjoiningRoomIndex);
			adjoiningRoom.AddNeighbour(corridorRoomIndex);
			m_RoomVec.push_back(adjoiningRoom);
			return true;
		}
	}

	return false;
}

void MapGenerator::RemoveRoomFromAllNeighbourLists(int roomIndex)
{
	for (int r = 0; r < Util::Size(m_RoomVec); ++r)
	{
		// function does nothing if not connected
		m_RoomVec[r].RemoveNeighbour(roomIndex);
	}
}

void MapGenerator::RenumberRoomInAllNeighbourLists(int oldRoomIndex, int newRoomIndex)
{
	for (int r = 0; r < Util::Size(m_RoomVec); ++r)
	{
		// function does nothing if not connected
		m_RoomVec[r].RenumberNeighbour(oldRoomIndex, newRoomIndex);
	}
}

bool MapGenerator::AreRoomsAlreadyConnected(int roomIndex1, int roomIndex2) const
{
	for (int n = 0; n < m_RoomVec[roomIndex1].GetNeighbourCount(); ++n)
	{
		int const neighbourIndex = m_RoomVec[roomIndex1].GetNeighbours()[n];

		//if (!m_RoomVec[neighbourIndex].IsCorridor())
		//	continue;  // only look for connections along corridors

		for (int n2 = 0; n2 < m_RoomVec[neighbourIndex].GetNeighbourCount(); ++n2)
		{
			if (m_RoomVec[neighbourIndex].GetNeighbours()[n2] == roomIndex2)
			{
				return true;
			}
		}
	}
	return false;
}

void MapGenerator::RemoveInvalidRoomsFromOptions(Room::TempList& options, bool check_border)
{
	// Remove invalid corridors from the list
	options.erase(std::remove_if(options.begin(), options.end(),
			[this,check_border](const Room &corridor)
			{
				return !IsValidRoom(corridor, check_border);
			}
		), options.cend());
}

void MapGenerator::RemoveBadlyPlacedStairsFromOptions(Room::TempList& options, Box2 otherMapBox)
{
	options.erase(std::remove_if(options.begin(), options.end(),
			[this, otherMapBox](const Room &new_stairs)
			{
				return IsBadlyPlacedStairs(new_stairs, otherMapBox);
			}
		), options.cend());
}

bool MapGenerator::IsBadlyPlacedStairs(Room const& newStairs, Box2 otherMapBox) const
{
	if (!m_Map.contains(newStairs.GetBox()) ||
		!otherMapBox.contains(newStairs.GetBox()))
	{
		return true;
	}

	// It's not good if it's going to run into the edge of the map.
	Stairs::Direction dir = newStairs.GetStairsDirection();
	Vec2 stairsStart = newStairs.StairsLocalEnd();
	Vec2 stairsVec = Stairs::relative_move(dir).xy();
	Vec2 stairsGoTowards = stairsStart + (m_Param.MaxRoomDimension + 2)*stairsVec;
	if (!otherMapBox.contains(stairsGoTowards))
	{
		return true;
	}

	// It's not good if it's too close to another stairs.
	for (Room const &room : m_RoomVec)
	{
		if (room.IsStairs())
		{
			if (AreStairsProblematic(newStairs, room))
			{
				return true;
			}
		}
	}

	return false;
}

bool MapGenerator::AreStairsProblematic(Room const& new_stairs, Room const& other_stairs) const
{
	Vec2 p0 = new_stairs.StairsLocalEnd();
	Vec2 p1 = other_stairs.StairsLocalEnd();

	if(strict_range(p0, p1, m_Param.MinStairsProximity))
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

void MapGenerator::MakeRoomARegionParent(int roomIndex)
{
	int const parentRegionIndex = m_RoomVec[roomIndex].GetRegion();

	// any regions that border this one get it as a parent

	for (int c = 0; c < m_RoomVec[roomIndex].GetNeighbourCount(); ++c)
	{
		int const otherIndex = m_RoomVec[roomIndex].GetNeighbours()[c];
		if (!m_RoomVec[otherIndex].IsInMainRegion() &&
			m_RoomVec[otherIndex].GetRegion() != parentRegionIndex)
		{
			int const childRegionIndex = m_RoomVec[otherIndex].GetRegion();
			m_RegionVec[childRegionIndex].parent = parentRegionIndex;
		}
	}
}

void MapGenerator::AddStairsToMap(Room const & room) const
{
	if (!room.IsStairs())
	{
		DebugBreak("Only use MapGenerator::AddStairsToMap for stairs.");
	}

	// Room positions are all in global space.

	// todo could probably make this better polymorphic design
	m_Map.add_stairs(room.StairsLocalEnd(), room.GetStairsDirection());

	// add a guard in room nearby
	Vec2 const backwards = Stairs::joining_vector(room.GetStairsDirection());
	Vec2 const near_foot = room.StairsLocalEnd() + backwards * 2;
	m_Map.edit_suggestions().add_enemy_weak(near_foot);
}

void MapGenerator::AddChamberToMap(Room const & room) const
{
	if (!room.IsChamber())
	{
		DebugBreak("Only use MapGenerator::AddChamberToMap for chmabers.");
	}

	// Room positions are all in global space.

	m_Map.fill_box(room.GetBox(), Terrain::Open);

	if (room.GetNeighbourCount() == 1)
	{
		// end room of region or 1-room attic

		// add treasure across the room from the entrance
		Vec2 pos = GetPosAtRoomBack(room);
		m_Map.edit_suggestions().add_treasure_normal(pos);
	}
	else if (room.GetRegion() != Room::c_MainRegion &&
	         room.GetNeighbourCount() >= 3)
	{
		// junction to several regions

		// add a guard
		Vec2 const roomCenter = room.GetBox().centre();
		m_Map.edit_suggestions().add_enemy_moderate(roomCenter);
	}
}

void MapGenerator::AddCorridorToMap(Room const & room) const
{
	if (!room.IsCorridor())
	{
		DebugBreak("Only use MapGenerator::AddCorridorToMap for corridors.");
	}

	// Room positions are all in global space.

	m_Map.fill_box(room.GetBox(), Terrain::Open);

	if (room.GetRegion() != Room::c_MainRegion &&
		room.GetNeighbourCount() == 2)  // false for T-junctions
	{
		Room const & neighbour0 = m_RoomVec[room.GetNeighbours()[0]];
		Room const & neighbour1 = m_RoomVec[room.GetNeighbours()[1]];

		int const parent_region = m_RegionVec[room.GetRegion()].parent;
		bool const is_edge_of_region_0 = neighbour0.GetRegion() == parent_region;
		bool const is_edge_of_region_1 = neighbour1.GetRegion() == parent_region;

		// the ends of the hallway aren't hard to find, but which is which?
		Vec2 door0 = room.GetBox().min;
		Vec2 door1 = room.GetBox().inner_max();
		Vec2 const neighbour0_Center = neighbour0.GetBox().centre();
		if (square_dist(neighbour0_Center, door1) <
			square_dist(neighbour0_Center, door0))
		{
			Vec2 temp = door0;
			door0 = door1;
			door1 = temp;
		}

		// secret passage - both ends locked
		if (is_edge_of_region_0 && is_edge_of_region_1 &&
		    room.CorridorLength() > 2)
		{
			// this can't happen yet...
			// TODO: Test when map generates secret passages
			PosTempList posList0 = GetEmptyWallPositions(neighbour0);
			PosTempList posList1 = GetEmptyWallPositions(neighbour1);
			if (Util::Size(posList0) > 0 &&
			    Util::Size(posList1) > 0)
			{
				Vec2 button_pos0 = posList0[Random::in_range(0, Util::Size(posList0) - 1)];
				Vec2 button_pos1 = posList1[Random::in_range(0, Util::Size(posList1) - 1)];
				if (Terrain::is_open(m_Map.get_terrain(button_pos0)) &&
				    Terrain::is_open(m_Map.get_terrain(button_pos1)))
				{
					m_Map.edit_suggestions().
						add_secret_passage(door0, door1, button_pos0, button_pos1);
					//m_Map.set_terrain(door0, Terrain::OpenAlternate);
					//m_Map.set_terrain(door1, Terrain::OpenAlternate);
					//m_Map.set_terrain(button_pos0, Terrain::OpenAlternate);
					//m_Map.set_terrain(button_pos1, Terrain::OpenAlternate);
					return;
				}
			}
			m_Map.edit_suggestions().add_secret_passage(door0, door1);
			//m_Map.set_terrain(door0, Terrain::OpenAlternate);
			//m_Map.set_terrain(door1, Terrain::OpenAlternate);
			return;
		}

		// secret area - only 1 end locked
		if (is_edge_of_region_0)
		{
			PosTempList posList = GetEmptyWallPositions(neighbour0);
			if (Util::Size(posList) > 0)
			{
				Vec2 button_pos = posList[Random::in_range(0, Util::Size(posList) - 1)];
				if (Terrain::is_open(m_Map.get_terrain(button_pos)))
				{
					m_Map.edit_suggestions().add_secret_area(door0, button_pos);
					//m_Map.set_terrain(door0, Terrain::OpenAlternate);
					//m_Map.set_terrain(button_pos, Terrain::OpenAlternate);
					return;
				}
			}
			m_Map.edit_suggestions().add_secret_area(door0);
			//m_Map.set_terrain(door0, Terrain::OpenAlternate);
		}
		if (is_edge_of_region_1)
		{
			PosTempList posList = GetEmptyWallPositions(neighbour1);
			if (Util::Size(posList) > 0)
			{
				Vec2 button_pos = posList[Random::in_range(0, Util::Size(posList) - 1)];
				if (Terrain::is_open(m_Map.get_terrain(button_pos)))
				{
					m_Map.edit_suggestions().add_secret_area(door1, button_pos);
					//m_Map.set_terrain(door1, Terrain::OpenAlternate);
					//m_Map.set_terrain(button_pos, Terrain::OpenAlternate);
					return;
				}
			}
			m_Map.edit_suggestions().add_secret_area(door1);
			//m_Map.set_terrain(door1, Terrain::OpenAlternate);
		}
	}

/*	// TODO doors
	if (room.CorridorLength() != 2 && !Random::one_in(3))
	{
		m_Map.set_terrain(room.GetBox().min, Terrain::Door);
		m_Map.set_terrain(room.GetBox().inner_max(), Terrain::Door);
	}*/
}

Vec2 MapGenerator::GetPosAtRoomBack(Room const & room) const
{
	Vec2 const roomCenter = room.GetBox().centre();

	int const neighbourIndex = room.GetNeighbours()[0];
	Vec2 const neighbourCenter = m_RoomVec[neighbourIndex].GetBox().centre();
	Vec2 const roomBackDirection = truncate_to_unit(roomCenter - neighbourCenter);

	// both components of roomBackDirection are -1, 0, or 1
	//  -> never (0, 0), so 8 possibilities
	// if 2 non-zero components: place in corner
	// if 1 zon-zero components: place at middle of wall

	Vec2 result = room.GetBox().min;
	switch (roomBackDirection.x)
	{
	// case -1: as initialized
	case  0:
		result.x = roomCenter.x;
		if(room.GetBox().size.x % 2 == 0 &&  // size is even
		   Random::coinflip())
		{
			// might choose other center-ish position
			result.x -= 1;
		}
		break;
	case  1:
		result.x = room.GetBox().max().x - 1;
		break;
	}

	switch (roomBackDirection.y)
	{
	// case -1: as initialized
	case  0:
		result.y = roomCenter.y;
		if(room.GetBox().size.y % 2 == 0 &&  // size is even
		   Random::coinflip())
		{
			// might choose other center-ish position
			result.y -= 1;
		}
		break;
	case  1:
		result.y = room.GetBox().max().y - 1;
		break;
	}

	return result;
}

MapGenerator::PosTempList MapGenerator::GetEmptyWallPositions(Room const & room) const
{
	// Goal: Find all positions
	//  1. Inside the room
	//  2. Along any wall
	//  3. Not in a corner
	//  4. Not by an attached corridor or stairs (including diagonally)

	PosTempList result_vec;

	// find excluded areas by corridors and stairs

	Box2TempList exclusion_vec;

	for (int n = 0; n < room.GetNeighbourCount(); ++n)
	{
		Box2 larger = m_RoomVec[room.GetNeighbours()[n]].GetBox();
		larger.min  -= {1, 1};
		larger.size += {2, 2};
		exclusion_vec.push_back(larger);
	}

	// find room edges

	int const x_min = room.GetBox().min.x;
	int const y_min = room.GetBox().min.y;
	int const x_max = room.GetBox().max(AXIS_X) - 1;
	int const y_max = room.GetBox().max(AXIS_Y) - 1;

	// search along X sides of room

	for (int y = y_min + 1; y < y_max; ++y)
	{
		// min X side
		Vec2 pos1{ x_min, y };
		if (!isContainedByAnyInList(pos1, exclusion_vec))
		{
			result_vec.push_back(pos1);
		}

		// max X side
		Vec2 pos2{ x_max, y };
		if (!isContainedByAnyInList(pos2, exclusion_vec))
		{
			result_vec.push_back(pos2);
		}
	}

	// search along Y sides of room

	for (int x = x_min + 1; x < x_max; ++x)
	{
		// min Y side
		Vec2 pos1{ x, y_min };
		if (!isContainedByAnyInList(pos1, exclusion_vec))
		{
			result_vec.push_back(pos1);
		}

		// max Y side
		Vec2 pos2{ x, y_max };
		if (!isContainedByAnyInList(pos2, exclusion_vec))
		{
			result_vec.push_back(pos2);
		}
	}

	// done
	return result_vec;
}

// static
bool MapGenerator::isContainedByAnyInList(Vec2 const & v, Box2TempList const & boxVec)
{
	for (int e = 0; e < Util::Size(boxVec); ++e)
	{
		if (boxVec[e].contains(v))
		{
			return true;
		}
	}
	return false;
}

void MapGenerator::PrintAllRooms() const
{
	std::cout << "All rooms" << std::endl;
	for (int r = 0; r < Util::Size(m_RoomVec); ++r)
	{
		Box2 const box = m_RoomVec[r].GetBox();
		Vec2 const box_max = box.min + box.size;
		switch (m_RoomVec[r].GetRoomType())
		{
		case RoomType::Chamber:          std::cout << "  Chamber "  << r << ":";  break;
		case RoomType::Corridor:         std::cout << "  Corridor " << r << ":";  break;
		case RoomType::Stairs:           std::cout << "  Stairs "   << r << ":";  break;
		case RoomType::IntermapCorridor: std::cout << "  Map-Corr " << r << ":";  break;
		default:
			std::cout << "  Unknown " << r << ":";
			break;
		}

		std::cout << "\t  (" << box.min.x << " - " << box_max.x
			<< ",\t" << box.min.y << " - " << box_max.y << ")";

		if(m_RoomVec[r].IsInMainRegion())
		{
			std::cout << "  \tMain region";
		}
		else
		{
			std::cout << "  \tRegion " << m_RoomVec[r].GetRegion();
		}

		std::cout << "\t{";
		for (int n = 0; n < m_RoomVec[r].GetNeighbourCount(); ++n)
		{
			int const neighbour = m_RoomVec[r].GetNeighbours()[n];
			std::cout << (n > 0 ? ", " : " ") << neighbour;
		}
		std::cout << " }" << std::endl;
	}
}
