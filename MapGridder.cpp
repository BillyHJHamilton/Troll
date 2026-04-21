#include "MapGridder.h"

#include "Debug.h"
#include "Feature.h"
#include "Map.h"
#include "MapGenerator.h"
#include "Random.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <cassert>

MapGridder::MapGridder(Map& map, MapGenerator& generator)
	: m_map(map)
	, m_generator(generator)
{
	// Room positions are all in global space.

	// Pass 0: Fill map with walls

	m_map.fill(Terrain::Wall);

	// Pass 1: Add basic rooms shapes
	//   -> including stairs
	//   -> it only safe to look in the grid representation for the current room

	for (int i = 0; i < m_generator.GetRoomCount(); ++i)
	{
		assert(m_map.contains(m_generator.GetRoom(i).GetBox()));
		add_basic(i);
	}

	// The grid representation now exists
	//   -> it can be safely queried
	//   -> we are now adding refinements rather than major changes
	//   -> functions add things in first-come-first-served order

	// Pass 2: Add important features
	//   -> secret areas and secret passages

	IntTempList index_list = Util::GetIndices(m_generator.GetRoomVector());
	Random::shuffle_vector(index_list);
	for (int index : index_list)
	{
		if (m_generator.GetRoom(index).IsCorridor())
		{
			add_corridor_doors(index);
		}
		// TODO: Add Fred and George here?
	}

	// Pass 3: Add cosmetic features
	//   -> armour, desks, cosmetic torches

	Random::shuffle_vector(index_list);
	for (int index : index_list)
	{
		if (m_generator.GetRoom(index).IsChamber())
		{
			add_cosmetic_chamber(index);
		}
	}

	// Do we want a spawn suggestions pass?

	// TODO: Pass 4: Add items
	//   -> and chests
	// Chests in dead-end rooms

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Converted map to grid, made {} suggestions.\n",
			m_map.read_suggestions().get_total_count());
	}
}


//-----------------------------------------------------------------------------
// Pass 1 function

void MapGridder::add_basic(int room_index) const
{
	Room const& room = m_generator.GetRoom(room_index);

	if (room.IsStairs())
	{
		// todo could probably make this better polymorphic design
		m_map.add_stairs(room.StairsLocalEnd(), room.GetStairsDirection());
		return;
	}

	if (m_generator.IsStartRoom(room_index))
	{
		// only the player starts here
		m_map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);
		m_map.edit_suggestions().add_player_start(room.GetBox().centre());
		return;
	}

	if (Terrain::c_HighlightType == Terrain::HighlightType::Regions &&
	    !room.IsInMainRegion())
	{
		m_map.fill_box(room.GetBox(), Terrain::OpenHighlight);
	}
	else
	{
		m_map.fill_box(room.GetBox(), Terrain::Open);
	}
}


//-----------------------------------------------------------------------------
// Pass 2 functions

void MapGridder::add_corridor_doors(int room_index) const
{
	Room const& room = m_generator.GetRoom(room_index);
	if (!room.IsCorridor())
	{
		DebugBreak("Only use MapGridder::add_corridor_doors for corridors.");
	}

	if (room.GetRegion() != Room::c_MainRegion &&
		room.GetNeighbourCount() == 2)  // false for T-junctions
	{
		// this corridor is not in the main region
		//  -> we might want to seal it off

		Room const & neighbour0 = m_generator.GetRoom(room.GetNeighbours()[0]);
		Room const & neighbour1 = m_generator.GetRoom(room.GetNeighbours()[1]);

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
		int const parent_region = m_generator.GetRegionParent(room.GetRegion());
		if (parent_region == Room::c_SecretPassage)
		{
			// don't spawn anything in the passage itself
			//  -> ends must be floor, so doors can spawn
			//    -> TODO: Can we avoid this?
			//  -> it's OK if things spawn in the door spots after the passage is open
			m_map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);
			//m_map.set_terrain(door0, Terrain::Open);
			//m_map.set_terrain(door1, Terrain::Open);

			add_secret_passage(room, neighbour0, door0, neighbour1, door1);
			return;
		}

		// secret area - only 1 end locked
		if (neighbour0.GetRegion() == parent_region)
		{
			m_map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);
			add_secret_area(room, neighbour0, door0);
			return;
		}
		else if (neighbour1.GetRegion() == parent_region)
		{
			m_map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);
			add_secret_area(room, neighbour1, door1);
			return;
		}
	}

/*	// TODO: doors?
	if (room.CorridorLength() != 2 && !Random::one_in(3))
	{
		m_Map.set_terrain(room.GetBox().min, Terrain::Door);
		m_Map.set_terrain(room.GetBox().inner_max(), Terrain::Door);
	}*/
}

void MapGridder::add_secret_area(Room const & room,
                                 Room const & neighbour, Vec2 const & door) const
{
	// TODO: Player start cannot be in a secret area
	// TODO: Spawn triggers and doors separately

	PosTempList button_pos_list = get_positions_inside_plain_wall(neighbour);
	PosTempList torch_pos_list  = choose_torch_positions(neighbour);

	// first bool is whether to allow triggerless doors
	bool is_allow_button = !button_pos_list.empty();
	bool is_allow_torch  = !torch_pos_list .empty();
	Spawn::TriggerType trigger_type = choose_trigger_type(true, is_allow_button, is_allow_torch);
	Spawn::DoorType       door_type = choose_door_type(trigger_type, /* allow none */ true);

	if (door_type == Spawn::DoorType::None)
	{
		return;  // no door
	}

	Terrain::Type door_terrain = get_terrain_for_door_type(door_type);
	int const map_z = m_map.get_z();

	switch(trigger_type)
	{
	case Spawn::TriggerType::None:
		Feature::spawn(door.xyz(map_z), door_terrain);
		break;

	case Spawn::TriggerType::FlipendoButton:
		{
			Vec2 button_pos = Random::from_vector(button_pos_list);
			Feature::spawn_flipendo_button(button_pos.xyz(map_z),
			                               door      .xyz(map_z), door_terrain);
		}
		break;

	case Spawn::TriggerType::LightTorch:
		if(Util::Size(torch_pos_list) == 1)
		{
			Feature::spawn_torch1_door(torch_pos_list[0].xyz(map_z),
			                           door  .xyz(map_z), door_terrain);
		}
		else if(Util::Size(torch_pos_list) == 4)
		{
			Feature::spawn_torch4_door(torch_pos_list[0].xyz(map_z),
			                           torch_pos_list[1].xyz(map_z),
			                           torch_pos_list[2].xyz(map_z),
			                           torch_pos_list[3].xyz(map_z),
			                           door  .xyz(map_z), door_terrain);
		}
		break;
	}
}

void MapGridder::add_secret_passage(Room const & room,
                                    Room const & neighbour0, Vec2 const & door0,
                                    Room const & neighbour1, Vec2 const & door1) const
{
	PosTempList button0_pos_list = get_positions_inside_plain_wall(neighbour0);
	PosTempList button1_pos_list = get_positions_inside_plain_wall(neighbour1);
	// TODO: Allow torches, only opens from one side?

	// first bool is whether to allow triggerless doors
	bool is_allow_button = !button0_pos_list.empty() && !button1_pos_list.empty();
	bool is_allow_torch  = false;
	Spawn::TriggerType trigger_type = choose_trigger_type(true, is_allow_button, is_allow_torch);
	Spawn::DoorType       door_type = choose_door_type(trigger_type, /* allow none */ false);

	assert(door_type != Spawn::DoorType::None);

	Terrain::Type door_terrain = get_terrain_for_door_type(door_type);
	int const map_z = m_map.get_z();

	switch(trigger_type)
	{
	case Spawn::TriggerType::None:
		Feature::spawn(door0.xyz(map_z), door_terrain);
		Feature::spawn(door1.xyz(map_z), door_terrain);
		break;

	case Spawn::TriggerType::FlipendoButton:
		{
			Vec2 button0_pos = Random::from_vector(button0_pos_list);
			Vec2 button1_pos = Random::from_vector(button1_pos_list);
			Feature::spawn_flipendo_button_pair(button0_pos.xyz(map_z),
			                                    door0      .xyz(map_z),
			                                    button1_pos.xyz(map_z),
			                                    door1      .xyz(map_z), door_terrain);
		}
		break;
	}
}

Spawn::TriggerType MapGridder::choose_trigger_type(bool allow_none,
                                                   bool allow_button,
                                                   bool allow_torch) const
{
	assert(allow_none || allow_button || allow_torch);

	Spawn::Parameters const& params = m_map.read_spawn_param();

	IntTempList trigger_weights((int)(Spawn::TriggerType::Count), 0);  // count, value

	if (allow_none)
	{
		int none_weight = params.trigger_weights[(int)(Spawn::TriggerType::None)];
		trigger_weights[(int)(Spawn::TriggerType::None)] = none_weight;
	}

	if (allow_button)
	{
		int button_weight = params.trigger_weights[(int)(Spawn::TriggerType::FlipendoButton)];
		trigger_weights[(int)(Spawn::TriggerType::FlipendoButton)] = button_weight;
	}

	if (allow_torch)
	{
		int torch_weight = params.trigger_weights[(int)(Spawn::TriggerType::LightTorch)];
		trigger_weights[(int)(Spawn::TriggerType::LightTorch)] = torch_weight;
	}

	return (Spawn::TriggerType)(Random::weighted_index(trigger_weights));
}

Spawn::DoorType MapGridder::choose_door_type(Spawn::TriggerType trigger_type, bool allow_none) const
{
	Spawn::Parameters const& params = m_map.read_spawn_param();

	IntTempList door_weights((int)(Spawn::DoorType::Count), 0);  // count, value

	for (int i = 0; i < Util::Size(door_weights); ++i)
	{
		if (i == (int)(Spawn::DoorType::None) && !allow_none)
		{
			continue;
		}
		if (!Spawn::is_compatible(trigger_type, (Spawn::DoorType)(i)))
		{
			continue;
		}

		door_weights[i] = params.door_weights[i];
	}

	return (Spawn::DoorType)(Random::weighted_index(door_weights));
}

// static
Terrain::Type MapGridder::get_terrain_for_door_type(Spawn::DoorType door_type)
{
	switch (door_type)
	{
	case Spawn::DoorType::Portrait:
		return Terrain::Portrait;
	case Spawn::DoorType::SlidingWall:
		return Terrain::SlidingWall;
	case Spawn::DoorType::Portcullis:
		return Terrain::Portcullis;
	default:
		return Terrain::Open;
	}
}


//-----------------------------------------------------------------------------
// Pass 3 functions

void MapGridder::add_cosmetic_chamber(int room_index) const
{
	Room const& room = m_generator.GetRoom(room_index);
	if (!room.IsChamber())
	{
		DebugBreak("Only use MapGridder::add_cosmetic_chamber for chambers.");
	}

	if (room.GetNeighbourCount() == 1)
	{
		// end room of region or 1-room attic

		// TODO: Add this with important stuff?
		//  -> add a placeholder terrain?

		Vec2 pos = get_pos_at_room_back(room);
		m_map.edit_suggestions().add_treasure_normal(pos);
	}

	// special room types
	// TODO: Remove these when we have vaults?
	switch(Random::in_range(0, 10))
	{
	case 0:
		add_cosmetic_torches(room);
		break;
	case 1:
	case 2:
		add_cosmetic_armour(room);
		break;
	case 3:
	case 4:
		add_cosmetic_desks(room);
		break;
	}

	if (room.GetRegion() == Room::c_MainRegion)
	{
		// boss should spawn in main region
		Vec2 const roomCenter = room.GetBox().centre();
		m_map.edit_suggestions().add_boss(roomCenter);

		if (Terrain::c_HighlightType == Terrain::HighlightType::Suggestions)
		{
			m_map.set_terrain(roomCenter, Terrain::OpenHighlight);
		}
	}
	else if (room.GetNeighbourCount() >= 3)
	{
		// junction to several regions

		// add a guard
		Vec2 const roomCenter = room.GetBox().centre();
		m_map.edit_suggestions().add_enemy_moderate(roomCenter);
	}
}

void MapGridder::add_cosmetic_torches(Room const& room) const
{
	// torches in the room are all lit or all unlit
	bool is_lit = Random::in_range(0, 99) < m_map.read_spawn_param().percent_torches_lit;
	Terrain::Type terrain = is_lit ? Terrain::TorchLit : Terrain::TorchUnlit;

	PosTempList positions =	choose_torch_positions(room);
	for (Vec2 pos : positions)
	{
		Feature::spawn(pos.xyz(m_map.get_z()), terrain);
	}
}

void MapGridder::add_cosmetic_armour(Room const & room) const
{
	// TODO: Armour flanking doorways sometimes

	int map_z = m_map.get_z();
	PosTempList positions =	get_positions_along_plain_wall(room);
	for (Vec2 pos : positions)
	{
		if ((pos.x + pos.y) % 2 == 0)  // every other space
		{
			Feature::spawn(pos.xyz(map_z), Terrain::Armour);
		}
	}
}

void MapGridder::add_cosmetic_desks(Room const& room) const
{
	// this box is 1 smaller on each side
	int const  min_x = room.GetBox().min.x + 1;
	int const  min_y = room.GetBox().min.y + 1;
	int const size_x = room.GetBox().size.x - 2;
	int const size_y = room.GetBox().size.y - 2;
	int const  max_x = room.GetBox().inner_max(AXIS_X);
	int const  max_y = room.GetBox().inner_max(AXIS_Y);

	if (size_x < 2 || size_y < 2)
	{
		return;  // room is too small for desks
	}

	bool is_aisle_x = size_x >= 5 && size_x % 2 != 0;
	bool is_aisle_y = size_y >= 5 && size_y % 2 != 0;

	if (is_aisle_x)
	{
		int half_x = size_x / 2;
		int min2_x = min_x + half_x + 1;

		if (is_aisle_y)
		{
			int half_y = size_y / 2;
			int min2_y = min_y + half_y + 1;

			add_desks_in_box(Box2{  min_x,  min_y, half_x, half_y });
			add_desks_in_box(Box2{  min_x, min2_y, half_x, half_y });
			add_desks_in_box(Box2{ min2_x,  min_y, half_x, half_y });
			add_desks_in_box(Box2{ min2_x, min2_y, half_x, half_y });
		}
		else
		{
			add_desks_in_box(Box2{  min_x, min_y, half_x, size_y });
			add_desks_in_box(Box2{ min2_x, min_y, half_x, size_y });
		}
	}
	else
	{
		if (is_aisle_y)
		{
			int half_y = size_y / 2;
			int min2_y = min_y + half_y + 1;

			add_desks_in_box(Box2{ min_x,  min_y, size_x, half_y });
			add_desks_in_box(Box2{ min_x, min2_y, size_x, half_y });
		}
		else
		{
			add_desks_in_box(Box2{ min_x, min_y, size_x, size_y });
		}
	}
}

void MapGridder::add_desks_in_box(Box2 const & box) const
{
	if (!is_good_floor(box.plus_border(1)))
	{
		return; // something is blocking us
	}

	int const map_z = m_map.get_z();
	int max_x = box.max(AXIS_X);
	int max_y = box.max(AXIS_Y);
	for (int x = box.min.x; x < max_x; ++x)
	{
		for (int y = box.min.y; y < max_y; ++y)
		{
			Feature::spawn(Vec3{ x, y, map_z }, Terrain::Desk);
		}
	}
}


//-----------------------------------------------------------------------------
// Functions to select positions

Vec2 MapGridder::get_pos_at_room_back(Room const& room) const
{
	Vec2 const roomCenter = room.GetBox().centre();

	int const neighbourIndex = room.GetNeighbours()[0];
	Vec2 const neighbourCenter = m_generator.GetRoom(neighbourIndex).GetBox().centre();
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

MapGridder::PosTempList MapGridder::choose_torch_positions(Room const& room) const
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
	// TODO: One torch at each end of the room (2 cases)
	else if (room.GetBox().size.x % 2 != 0 &&
	         room.GetBox().size.y % 2 != 0)
	{
		// 1 torch in center
		result.push_back(room.GetBox().centre());
	}

	if (!result.empty() && is_good_for_isolated_floor(result))
	{
		// if we found good torch spots, use those
		return result;
	}

	// 1 torch along the wall at random
	result.clear();
	PosTempList possible = get_positions_along_plain_wall(room);
	if (Util::Size(possible) > 0)
	{
		result.push_back(Random::from_vector(possible));
	}
	return result;
}

MapGridder::PosTempList MapGridder::get_positions_along_plain_wall(Room const& room) const
{
	// Goal: Find all positions
	//  1. Inside the room
	//  2. Along any wall
	//  3. Not in a corner
	//  4. On a piece of isolated floor
	//    -> We check the terrain for this

	PosTempList result_vec;

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
		if (is_by_west_wall(pos1) &&
			is_good_for_isolated_floor(pos1))
		{
			result_vec.push_back(pos1);
		}

		// max X side
		Vec2 pos2{ x_max, y };
		if (is_by_east_wall(pos2) &&
			is_good_for_isolated_floor(pos2))
		{
			result_vec.push_back(pos2);
		}
	}

	// search along Y sides of room

	for (int x = x_min + 1; x < x_max; ++x)
	{
		// min Y side
		Vec2 pos1{ x, y_min };
		if (is_by_north_wall(pos1) &&
			is_good_for_isolated_floor(pos1))
		{
			result_vec.push_back(pos1);
		}

		// max Y side
		Vec2 pos2{ x, y_max };
		if (is_by_south_wall(pos2) &&
			is_good_for_isolated_floor(pos2))
		{
			result_vec.push_back(pos2);
		}
	}

	// done
	return result_vec;
}

MapGridder::PosTempList MapGridder::get_positions_inside_plain_wall(Room const& room) const
{
	// Goal: Find all positions
	//  1. Inside the room wall
	//  2. With open space in front
	//  3. That are surrounded by wall on 5 sides, including 2 diagonally
	//
	// This has to be a separate function from get_positions_along_plain_wall.
	//  -> The requirements are too different.

	PosTempList result_vec;

	// find room edges

	int const inside_x_min = room.GetBox().min.x;
	int const inside_y_min = room.GetBox().min.y;
	int const inside_x_max = room.GetBox().inner_max(AXIS_X);
	int const inside_y_max = room.GetBox().inner_max(AXIS_Y);

	// search along min X (west) side of room

	int const wall_x_min = inside_x_min - 1;
	if (m_map.contains(Box2(wall_x_min - 1, inside_y_min, 3, room.GetBox().size.y)))
	{
		// we are inside the map
		for (int y = inside_y_min; y <= inside_y_max; ++y)
		{
			Vec2 pos1{ wall_x_min, y };
			if (is_good_floor(Vec2{ inside_x_min, y }) &&
				is_inside_north_south_wall(pos1) &&
				is_by_west_wall(pos1))
			{
				result_vec.push_back(pos1);
			}
		}
	}

	// search along max X (east) side of room

	int const wall_x_max = inside_x_max + 1;
	if (m_map.contains(Box2(wall_x_max - 1, inside_y_min, 3, room.GetBox().size.y)))
	{
		// we are inside the map
		for (int y = inside_y_min; y <= inside_y_max; ++y)
		{
			Vec2 pos2{ wall_x_max, y };
			if (is_good_floor(Vec2{ inside_x_min, y }) &&
				is_inside_north_south_wall(pos2) &&
				is_by_east_wall(pos2))
			{
				result_vec.push_back(pos2);
			}
		}
	}

	// search along min Y (north) side of room

	int const wall_y_min = inside_y_min - 1;
	if (m_map.contains(Box2(inside_x_min, wall_y_min - 1, room.GetBox().size.x, 3)))
	{
		// we are inside the map
		for (int x = inside_x_min; x <= inside_x_max; ++x)
		{
			Vec2 pos1{ x, wall_y_min };
			if (is_good_floor(Vec2{ x, inside_y_min }) &&
				is_inside_east_west_wall(pos1) &&
				is_by_north_wall(pos1))
			{
				result_vec.push_back(pos1);
			}
		}
	}

	// search along max Y (south) side of room

	int const wall_y_max = inside_y_max + 1;
	if (m_map.contains(Box2(inside_x_min, wall_y_max - 1, room.GetBox().size.x, 3)))
	{
		// we are inside the map
		for (int x = inside_x_min; x <= inside_x_max; ++x)
		{
			Vec2 pos2{ x, wall_y_min };
			if (is_good_floor(Vec2{ x, inside_y_min }) &&
				is_inside_east_west_wall(pos2) &&
				is_by_south_wall(pos2))
			{
				result_vec.push_back(pos2);
			}
		}
	}

	// done
	return result_vec;
}


//-----------------------------------------------------------------------------
// Functions to check if positions are good

bool MapGridder::is_good_for_spawn(Vec2 const& pos) const
{
	Terrain::Type terrain = m_map.get_terrain(pos);
	return terrain == Terrain::Wall || Terrain::is_can_spawn(terrain);
}

bool MapGridder::is_good_floor(Vec2 const & pos) const
{
	return Terrain::is_can_spawn(m_map.get_terrain(pos));
}

bool MapGridder::is_good_wall(Vec2 const & pos) const
{
	return m_map.get_terrain(pos) == Terrain::Wall;
}

bool MapGridder::is_good_neighbours(Vec2 const& pos) const
{
	return
		is_good_for_spawn(Vec2{ pos.x - 1, pos.y     }) &&
		is_good_for_spawn(Vec2{ pos.x + 1, pos.y     }) &&
		is_good_for_spawn(Vec2{ pos.x,     pos.y - 1 }) &&
		is_good_for_spawn(Vec2{ pos.x,     pos.y + 1 }) &&
		is_good_for_spawn(Vec2{ pos.x - 1, pos.y - 1 }) &&
		is_good_for_spawn(Vec2{ pos.x - 1, pos.y + 1 }) &&
		is_good_for_spawn(Vec2{ pos.x + 1, pos.y - 1 }) &&
		is_good_for_spawn(Vec2{ pos.x + 1, pos.y + 1 });
}

bool MapGridder::is_good_for_isolated_floor(Vec2 const& pos) const
{
	return is_good_floor(pos) && is_good_neighbours(pos);
}

bool MapGridder::is_good_for_isolated_wall(Vec2 const& pos) const
{
	return is_good_wall(pos) && is_good_neighbours(pos);
}

bool MapGridder::is_good_for_isolated_floor(PosTempList const& pos_list) const
{
	for (Vec2 const& pos : pos_list)
	{
		if (!is_good_for_isolated_floor(pos))
		{
			return false;
		}
	}
	return true;
}

bool MapGridder::is_good_floor(Box2 const& box) const
{
	int max_x = box.max(AXIS_X);
	int max_y = box.max(AXIS_Y);
	for (int x = box.min.x; x < max_x; ++x)
	{
		for (int y = box.min.y; y < max_y; ++y)
		{
			if (!is_good_floor(Vec2{ x, y }))
			{
				return false;
			}
		}
	}
	return true;
}

bool MapGridder::is_inside_east_west_wall(Vec2 const & pos) const
{
	return
		is_good_wall(Vec2{ pos.x - 1, pos.y }) &&
		is_good_wall(Vec2{ pos.x,     pos.y }) &&
		is_good_wall(Vec2{ pos.x + 1, pos.y });
}

bool MapGridder::is_inside_north_south_wall(Vec2 const & pos) const
{
	return
		is_good_wall(Vec2{ pos.x, pos.y - 1 }) &&
		is_good_wall(Vec2{ pos.x, pos.y     }) &&
		is_good_wall(Vec2{ pos.x, pos.y + 1 });
}

/*
	// Functions to select positions in or near rooms
	PosTempList GetPlainWallPositions(Room const & room) const;
	static bool isContainedByAnyInList(Vec2 const & pos, Box2TempList const & boxVec);
	static bool isAnyContainedByAnyInList(PosTempList const & posVec,
	                                      Box2TempList const & boxVec);
 */
