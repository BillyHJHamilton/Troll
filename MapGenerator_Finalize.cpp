#include "MapGenerator.h"

#include "Debug.h"
#include "Map.h"
#include "Random.h"
#include "Terrain.h"
#include "VectorUtil.h"

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

// Helpers for AddAllToMap

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

	if (Terrain::c_HighlightType == Terrain::HighlightType::Regions &&
	    !room.IsInMainRegion())
	{
		m_Map.fill_box(room.GetBox(), Terrain::OpenHighlight);
	}
	else
	{
		m_Map.fill_box(room.GetBox(), Terrain::Open);
	}

	if (room.GetRegion() == Room::c_MainRegion)
	{
		// boss should spawn in main region
		Vec2 const roomCenter = room.GetBox().centre();
		m_Map.edit_suggestions().add_boss(roomCenter);

		if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
		{
			m_Map.set_terrain(roomCenter, Terrain::OpenHighlight);
		}
	}

	if (room.GetNeighbourCount() == 1)
	{
		// end room of region or 1-room attic

		// add treasure across the room from the entrance
		Vec2 pos = GetPosAtRoomBack(room);
		m_Map.edit_suggestions().add_treasure_normal(pos);
	}
	else
	{
		// special room types
		// TODO: Remove these when we have vaults?
		switch(Random::in_range(0, 10))
		{
		case 0:
		case 1:
			AddDeskRoomSuggestions(room);
			break;
		case 2:
		case 3:
			AddArmourRoomSuggestions(room);
			break;
		case 4:
			AddCosmeticTorchRoomSuggestions(room);
			break;
		}

		if (room.GetRegion() != Room::c_MainRegion &&
			room.GetNeighbourCount() >= 3)
		{
			// junction to several regions

			// add a guard
			Vec2 const roomCenter = room.GetBox().centre();
			m_Map.edit_suggestions().add_enemy_moderate(roomCenter);
		}
	}
}

void MapGenerator::AddCorridorToMap(Room const & room) const
{
	if (!room.IsCorridor())
	{
		DebugBreak("Only use MapGenerator::AddCorridorToMap for corridors.");
	}

	// Room positions are all in global space.

	if (Terrain::c_HighlightType == Terrain::HighlightType::Regions &&
	    !room.IsInMainRegion())
	{
		m_Map.fill_box(room.GetBox(), Terrain::OpenHighlight);
	}
	else
	{
		m_Map.fill_box(room.GetBox(), Terrain::Open);
	}

	if (room.GetRegion() != Room::c_MainRegion &&
		room.GetNeighbourCount() == 2)  // false for T-junctions
	{
		Room const & neighbour0 = m_RoomVec[room.GetNeighbours()[0]];
		Room const & neighbour1 = m_RoomVec[room.GetNeighbours()[1]];

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
		int const parent_region = m_RegionVec[room.GetRegion()].parent;
		if (parent_region == Room::c_SecretPassage)
		{
			// don't spawn anything in the passage itself
			//  -> ends must be floor, so doors can spawn
			//  -> it's OK if things spawn in the door spots after the passage is open
			m_Map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);
			m_Map.set_terrain(door0, Terrain::Open);
			m_Map.set_terrain(door1, Terrain::Open);

			AddSecretPassageSuggestions(room, neighbour0, door0, neighbour1, door1);
			return;
		}

		// secret area - only 1 end locked
		if (neighbour0.GetRegion() == parent_region)
		{
			AddSecretAreaSuggestions(room, neighbour0, door0);
			return;
		}
		else if (neighbour1.GetRegion() == parent_region)
		{
			AddSecretAreaSuggestions(room, neighbour1, door1);
			return;
		}
	}

/*	// TODO doors
	if (room.CorridorLength() != 2 && !Random::one_in(3))
	{
		m_Map.set_terrain(room.GetBox().min, Terrain::Door);
		m_Map.set_terrain(room.GetBox().inner_max(), Terrain::Door);
	}*/
}

void MapGenerator::AddDeskRoomSuggestions(Room const & room) const
{
	int const  min_x = room.GetBox().min.x;
	int const  min_y = room.GetBox().min.y;
	int const size_x = room.GetBox().size.x;
	int const size_y = room.GetBox().size.y;
	int const  max_x = room.GetBox().max(AXIS_X);
	int const  max_y = room.GetBox().max(AXIS_Y);

	if (size_x < 4 || size_y < 4)
	{
		return;  // room is too small for desks
	}

	bool is_aisle_x = size_x >= 7 && size_x % 2 != 0;
	bool is_aisle_y = size_y >= 7 && size_y % 2 != 0;

	if (is_aisle_x)
	{
		int half_x = (size_x + 1) / 2;
		int min2_x = min_x + half_x - 1;

		if (is_aisle_y)
		{
			int half_y = (size_y + 1) / 2;
			int min2_y = min_y + half_y - 1;

			m_Map.edit_suggestions().add_desk_block(Box2{  min_x,  min_y, half_x, half_y });
			m_Map.edit_suggestions().add_desk_block(Box2{  min_x, min2_y, half_x, half_y });
			m_Map.edit_suggestions().add_desk_block(Box2{ min2_x,  min_y, half_x, half_y });
			m_Map.edit_suggestions().add_desk_block(Box2{ min2_x, min2_y, half_x, half_y });
		}
		else
		{
			m_Map.edit_suggestions().add_desk_block(Box2{  min_x, min_y, half_x, size_y });
			m_Map.edit_suggestions().add_desk_block(Box2{ min2_x, min_y, half_x, size_y });
		}
	}
	else
	{
		if (is_aisle_y)
		{
			int half_y = (size_y + 1) / 2;
			int min2_y = min_y + half_y - 1;

			m_Map.edit_suggestions().add_desk_block(Box2{ min_x,  min_y, size_x, half_y });
			m_Map.edit_suggestions().add_desk_block(Box2{ min_x, min2_y, size_x, half_y });
		}
		else
		{
			m_Map.edit_suggestions().add_desk_block(Box2{ min_x, min_y, size_x, size_y });
		}
	}
}

void MapGenerator::AddCosmeticTorchRoomSuggestions(Room const & room) const
{
	// all torches in the room have the same random value
	//  -> this will be compared as chosen < desired, so 100 is impossible
	int random_percent = Random::in_range(0, 99);

	PosTempList positions =	GetTorchPositions(room);
	for (Vec2 pos : positions)
	{
		m_Map.edit_suggestions().add_cosmetic_torch(pos, random_percent);
	}
}

void MapGenerator::AddArmourRoomSuggestions(Room const & room) const
{
	// TODO: Armour flanking doorways sometimes

	PosTempList positions =	GetPositionsAlongPlainWall(room);
	for (Vec2 pos : positions)
	{
		if ((pos.x + pos.y) % 2 == 0)  // every other space
		{
			m_Map.edit_suggestions().add_armour(pos);
		}
	}
}

void MapGenerator::AddSecretPassageSuggestions(Room const & room,
                                               Room const & neighbour0, Vec2 const & door0,
                                               Room const & neighbour1, Vec2 const & door1) const
{
	PosTempList posList0 = GetPlainWallPositions(neighbour0);
	PosTempList posList1 = GetPlainWallPositions(neighbour1);
	if (Util::Size(posList0) > 0 &&
		Util::Size(posList1) > 0)
	{
		// add with buttons

		Vec2 button_pos0 = Random::from_vector(posList0);
		Vec2 button_pos1 = Random::from_vector(posList1);
		m_Map.edit_suggestions().add_secret_passage(door0, door1, button_pos0, button_pos1);

		if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
		{
			m_Map.set_terrain(door0, Terrain::OpenHighlight);
			m_Map.set_terrain(door1, Terrain::OpenHighlight);
			m_Map.set_terrain(button_pos0, Terrain::OpenHighlight);
			m_Map.set_terrain(button_pos1, Terrain::OpenHighlight);
		}
	}
	else
	{
		// add without buttons

		m_Map.edit_suggestions().add_secret_passage(door0, door1);

		if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
		{
			m_Map.set_terrain(door0, Terrain::OpenHighlight);
			m_Map.set_terrain(door1, Terrain::OpenHighlight);
		}
	}
}

void MapGenerator::AddSecretAreaSuggestions(Room const & room,
                                            Room const & neighbour, Vec2 const & door) const
{
	PosTempList button_pos_list = GetPlainWallPositions(neighbour);
	PosTempList torch_pos_list  = GetTorchPositions(neighbour);

	if (Util::Size(button_pos_list) > 0 &&
		Util::Size(torch_pos_list) > 0)
	{
		Vec2 button_pos = Random::from_vector(button_pos_list);

		// What if 2 secret doors spawn buttons in the same place?
		//  -> Currently caught in Spawn
		//  -> 2nd door spawned will be of a buttonless type (e.g. Portrait)
		//  -> Do we want to check for collisions here?

		// TODO: pass torch positions as vector?
		//  -> 2 torches in a set (3xN rooms)
		//  -> And farther along, so we can do any number?
		if (Util::Size(torch_pos_list) == 1)
		{
			m_Map.edit_suggestions().add_secret_area(door, button_pos, torch_pos_list[0]);
		}
		else
		{
			assert(Util::Size(torch_pos_list) == 4);
			m_Map.edit_suggestions().add_secret_area(door, button_pos,
				torch_pos_list[0], torch_pos_list[1], torch_pos_list[2], torch_pos_list[3]);
		}

		if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
		{
			m_Map.set_terrain(door, Terrain::OpenHighlight);
			m_Map.set_terrain(button_pos, Terrain::OpenHighlight);
		}
		return;
	}

	m_Map.edit_suggestions().add_secret_area(door);
	if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
	{
		m_Map.set_terrain(door, Terrain::OpenHighlight);
	}
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

MapGenerator::PosTempList MapGenerator::GetTorchPositions(Room const & room) const
{
	PosTempList result;

	if (room.GetBox().size.x >= 5 &&
		room.GetBox().size.y >= 5)
	{
		// 4 torches in corners
		Vec2 const pos_min = room.GetBox().min         + Vec2{ 1, 1 };
		Vec2 const pos_max = room.GetBox().inner_max() - Vec2{ 1, 1 };
		result.push_back(Vec2{ pos_min.x, pos_min.y });
		result.push_back(Vec2{ pos_min.x, pos_max.y });
		result.push_back(Vec2{ pos_max.x, pos_min.y });
		result.push_back(Vec2{ pos_max.x, pos_max.y });
	}
	else if (room.GetBox().size.x % 2 != 0 &&
	         room.GetBox().size.y % 2 != 0)
	{
		// 1 torch in center
		result.push_back(room.GetBox().centre());
	}
	else
	{
		// 1 torch along the wall
		PosTempList possible = GetPositionsAlongPlainWall(room);
		if (Util::Size(possible) > 0)
		{
			result.push_back(Random::from_vector(possible));
		}
	}

	return result;
}

MapGenerator::PosTempList MapGenerator::GetPositionsAlongPlainWall(Room const & room) const
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
		Box2 const larger = m_RoomVec[room.GetNeighbours()[n]].GetBox();
		exclusion_vec.push_back(larger.plus_border(1));
	}

	// find room edges

	int const x_min = room.GetBox().min.x;
	int const y_min = room.GetBox().min.y;
	int const x_max = room.GetBox().inner_max(AXIS_X);
	int const y_max = room.GetBox().inner_max(AXIS_Y);

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

MapGenerator::PosTempList MapGenerator::GetPlainWallPositions(Room const & room) const
{
	// Goal: Find all positions
	//  1. Inside the room wall
	//  2. That are surrounded by wall on 5 sides, including 2 diagonally
	//    -> this requires checking all rooms, not just neighbours (oh no!)
	//
	// This has to be a separate function from GetEmptyPositionsAlongWall.
	//  -> The requirements are too different.

	PosTempList result_vec;

	// find excluded areas by corridors and stairs

	Box2TempList exclusion_vec;

	for (int r = 0; r < Util::Size(m_RoomVec); ++r)
	{
		if (&room == &(m_RoomVec[r]))
		{
			continue;  // skip this room
		}

		Box2 const larger = m_RoomVec[r].GetBox();
		exclusion_vec.push_back(larger.plus_border(1));
	}

	// find room edges

	int const inside_x_min = room.GetBox().min.x;
	int const inside_y_min = room.GetBox().min.y;
	int const inside_x_max = room.GetBox().inner_max(AXIS_X);
	int const inside_y_max = room.GetBox().inner_max(AXIS_Y);

	// search along X sides of room

	int const wall_x_min = inside_x_min - 1;
	int const wall_x_max = inside_x_max + 1;

	for (int y = inside_y_min; y <= inside_y_max; ++y)
	{
		// min X side
		Vec2 pos1{ wall_x_min, y };
		if (!isContainedByAnyInList(pos1, exclusion_vec))
		{
			result_vec.push_back(pos1);
		}

		// max X side
		Vec2 pos2{ wall_x_max, y };
		if (!isContainedByAnyInList(pos2, exclusion_vec))
		{
			result_vec.push_back(pos2);
		}
	}

	// search along Y sides of room

	int const wall_y_min = inside_y_min - 1;
	int const wall_y_max = inside_y_max + 1;

	for (int x = inside_x_min; x <= inside_x_max; ++x)
	{
		// min Y side
		Vec2 pos1{ x, wall_y_min };
		if (!isContainedByAnyInList(pos1, exclusion_vec))
		{
			result_vec.push_back(pos1);
		}

		// max Y side
		Vec2 pos2{ x, wall_y_max };
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

// static
bool MapGenerator::isAnyContainedByAnyInList(PosTempList const & posVec,
                                             Box2TempList const & boxVec)
{
	for (int e = 0; e < Util::Size(posVec); ++e)
	{
		if (isContainedByAnyInList(posVec[e], boxVec))
		{
			return true;
		}
	}
	return false;
}
