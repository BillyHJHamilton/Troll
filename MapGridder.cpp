#include "MapGridder.h"

#include "Debug.h"
#include "Feature.h"
#include "Map.h"
#include "MapGenerator.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <cassert>

MapGridder::MapGridder(Map& map, MapGenerator& generator, int map_id)
	: m_map(map)
	, m_generator(generator)
	, m_doors(map.read_door_param())
{
	PerfTimer perf0("map to grid");

	// Room positions are all in global space.

	// Pass 0: Fill map with walls

	m_map.fill(Terrain::Wall);

	// Pass 1: Add basic rooms shapes
	//   -> including stairs
	//   -> it's only safe to look in the grid representation for the current room

	for (int i = 0; i < m_generator.GetRoomCount(); ++i)
	{
		assert(m_map.contains(m_generator.GetRoom(i).GetBox()));
		add_basic(i);
	}

	// The grid representation now exists
	//   -> it can be safely queried anywhere
	//   -> we are now adding refinements rather than major changes
	//   -> open space on the map is available for any function to use
	//     -> first come, first served
	//     -> once used, turn it into some other terrain
	//     -> don't replace terrain that isn't "good" (can_spawn floor and walls)
	//     -> to reserve open floor space for later use, replace it with Terrain::Placeholder

	// Pass 2: Add important features
	//   -> choose chest locations (chests will be added with Items)
	//   -> shop location
	//   -> secret areas and secret passages

	add_treasure_suggestions();

	if (generator.ReadParameters().IsShopSeed)
	{
		add_shop_seed();
	}

	IntTempList index_list = Util::GetIndices(m_generator.GetRoomVector());
	Random::shuffle_vector(index_list);
	for (int index : index_list)
	{
		if (m_generator.GetRoom(index).IsCorridor())
		{
			add_corridor_doors(index);
		}
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

	Random::shuffle_vector(index_list);
	for (int index : index_list)
	{
		if (m_generator.GetRoom(index).IsCorridor())
		{
			add_unlocked_doors(index);
		}
	}

	// Do we want a spawn suggestions pass?

	// Pass 4: Add items (and chests)

	Spawn::spawn_early(m_map, map_id);
	replace_all(Terrain::Placeholder, Terrain::Open);

	// Done

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Converted map to grid, made {} suggestions.\n",
			m_map.read_suggestions().get_total_count());
	}
}


//-----------------------------------------------------------------------------
// Pass 1 functions

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

void MapGridder::add_treasure_suggestions() const
{
	// any order is fine
	for (Room const& room : m_generator.GetRoomVector())
	{
		if (!room.IsChamber())
		{
			continue;
		}

		// TODO: Ignore secret passges when counting exits?
		if (room.GetNeighbourCount() == 1)
		{
			// end room of region or 1-room attic

			Vec2 pos = get_pos_at_room_back(room);
			m_map.edit_suggestions().add_treasure_normal(pos);
			m_map.set_terrain(pos, Terrain::Placeholder);
		}
	}
}

void MapGridder::add_shop_seed() const
{
	IntTempList room_index_list = Util::GetIndices(m_generator.GetRoomVector());
	Random::shuffle_vector(room_index_list);
	for (int room_index : room_index_list)
	{
		Room const& room = m_generator.GetRoom(room_index);
		if (!room.IsChamber() || !room.IsInMainRegion())
		{
			continue;
		}

		PosTempList const pos_vec = box_to_positions(room.GetBox().minus_border(1));
		IntTempList pos_index_list = Util::GetIndices(pos_vec);
		Random::shuffle_vector(pos_index_list);
		for (int pos_index : pos_index_list)
		{
			Vec2 const& shop_centre = pos_vec[pos_index];
			Box2 shop_area = Box2::around_tile(shop_centre, 1);
			if (is_good_floor(shop_area))
			{
				m_map.fill_box(shop_area, Terrain::OpenNoSpawn);

				int map_z = m_map.get_z();
				Feature::spawn(shop_centre.xyz(map_z), Terrain::ShopSeed);

				//int trigger = Feature::get_new_trigger();
				//Vec2 scan_from = shop_centre + c_Compass[Random::compass_direction(false)];
				//Feature::spawn(scan_from  .xyz(map_z), Terrain::Scanner,  trigger);
				//Feature::spawn(shop_centre.xyz(map_z), Terrain::ShopSeed, trigger);

				if (Debug::enabled(Debug::Map))
				{
					std::cout << "Added shop seed.\n";
				}
				return;  // only 1 shop
			}
		}
	}
}

void MapGridder::add_corridor_doors(int room_index) const
{
	Room const& room = m_generator.GetRoom(room_index);
	if (!room.IsCorridor())
	{
		DebugBreak("Only use MapGridder::add_corridor_doors for corridors.");
	}

	if (room.GetRegion() != Room::c_MainRegion &&
		room.GetNeighbourCount() == 2 &&  // false for T-junctions
		!m_generator.IsStartRegionOrAncestorOfIt(room.GetRegion()))
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
			//  -> it's OK if things spawn where the doors were after the passage is open
			m_map.fill_box(room.GetBox(), Terrain::OpenNoSpawn);

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
}

void MapGridder::add_secret_area(Room const & room,
                                 Room const & neighbour, Vec2 const & door) const
{
	// TODO: Doors on both ends of the entrace corridor?
	//  -> secret passages let you reach the secret area from the wrong end
	//  -> with triggers, both would open together
	//  -> I think add_unlocked_door might now add one anyway

	int const map_z = m_map.get_z();

	PosTempList const button_pos_list = get_good_positions_inside_wall(neighbour);
	PosTempList const torch_pos_list  = choose_torch_positions(neighbour);

	bool const is_allow_button = !button_pos_list.empty();
	bool const is_allow_torch  = ! torch_pos_list.empty();
	Door::TriggerType const trigger_type =
		m_doors.choose_trigger_type(is_allow_button, is_allow_torch);

	bool const is_allow_trigger = trigger_type != Door::TriggerType::NotPossible;
	switch(m_doors.choose_locked_genus(/* allow none */ true, is_allow_trigger))
	{
		case Door::LockedGenus::Spell:
		{
			Door::Spelled const door_type = m_doors.choose_spelled();
			Terrain::Type const door_terrain = Door::get_terrain(door_type);
			Feature::spawn(door.xyz(map_z), door_terrain);
			break;
		}

		case Door::LockedGenus::Trigger:
		{
			int const trigger = Feature::get_new_trigger();

			Door::Triggered const door_type = m_doors.choose_triggered();
			Terrain::Type   const door_terrain = Door::get_terrain(door_type);
			Feature::spawn(door.xyz(map_z), door_terrain, trigger);

			switch (trigger_type)
			{
				case Door::TriggerType::FlipendoButton:
				{
					Vec2 const button = Random::from_vector(button_pos_list);
					Feature::spawn(button.xyz(map_z), Terrain::FlipendoButton, trigger);
					break;
				}
				case Door::TriggerType::LightTorch:
				{
					for (int i = 0; i < Util::Size(torch_pos_list); ++i)
					{
						Feature::spawn(torch_pos_list[i].xyz(map_z), Terrain::TorchUnlit, trigger);
					}
					break;
				}
			}

			// TODO: Could sometimes add a fire crab trap (if there is a trigger)
			//  -> it would appear in the neighbour room
			break;
		}
	}
}

void MapGridder::add_secret_passage(Room const & room,
                                    Room const & neighbour0, Vec2 const & door0,
                                    Room const & neighbour1, Vec2 const & door1) const
{
	int const map_z = m_map.get_z();

	PosTempList button0_pos_list = get_good_positions_inside_wall(neighbour0);
	PosTempList button1_pos_list = get_good_positions_inside_wall(neighbour1);

	// Can trigger on torches, but then the passage only opens from one side
	PosTempList torch_pos_list;
	if (Random::coinflip())
	{
		torch_pos_list = choose_torch_positions(neighbour0);
	}
	else
	{
		torch_pos_list = choose_torch_positions(neighbour1);
	}

	bool const is_allow_button = !button0_pos_list.empty() && !button1_pos_list.empty();
	bool const is_allow_torch  = !torch_pos_list.empty();
	Door::TriggerType const trigger_type = m_doors.choose_trigger_type(is_allow_button, is_allow_torch);

	bool const is_allow_trigger = trigger_type != Door::TriggerType::NotPossible;
	switch(m_doors.choose_locked_genus(/* allow none */ false, is_allow_trigger))
	{
		case Door::LockedGenus::Spell:
		{
			Door::Spelled const door_type = m_doors.choose_spelled();
			Terrain::Type const door_terrain = Door::get_terrain(door_type);
			Feature::spawn(door0.xyz(map_z), door_terrain);
			if (room.CorridorLength() >= 3)
			{
				if (door_type == Door::Spelled::AlohamoraDoor)
				{
					// 2 locked doors in a row is annoying because one blocks LoS to the other
					Feature::spawn(door1.xyz(map_z), Terrain::DoorOpen);
				}
				else
				{
					Feature::spawn(door1.xyz(map_z), door_terrain);
				}
			}
			break;
		}

		case Door::LockedGenus::Trigger:
		{
			int const trigger = Feature::get_new_trigger();

			Door::Triggered const door_type = m_doors.choose_triggered();
			Terrain::Type   const door_terrain = Door::get_terrain(door_type);
			Feature::spawn(door0.xyz(map_z), door_terrain, trigger);
			if (room.CorridorLength() > 1)
			{
				Feature::spawn(door1.xyz(map_z), door_terrain, trigger);
			}

			switch (trigger_type)
			{
				case Door::TriggerType::FlipendoButton:
				{
					Vec2 const button0 = Random::from_vector(button0_pos_list);
					Vec2 const button1 = Random::from_vector(button1_pos_list);
					Feature::spawn(button0.xyz(map_z), Terrain::FlipendoButton, trigger);
					Feature::spawn(button1.xyz(map_z), Terrain::FlipendoButton, trigger);
					break;
				}
				case Door::TriggerType::LightTorch:
				{
					for (int i = 0; i < Util::Size(torch_pos_list); ++i)
					{
						Feature::spawn(torch_pos_list[i].xyz(map_z), Terrain::TorchUnlit, trigger);
					}
					break;
				}
			}
			break;
		}
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

	// special room types
	// TODO: Remove these when we have vaults?
	//  -> or at least use a heavier-duty system
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
	bool is_lit = Random::in_range(0, 99) < m_generator.ReadParameters().percent_torches_lit;
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
	PosTempList positions =	get_good_positions_along_wall(room);
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
	int const  max_x = room.GetBox().inner_max(c_AxisX);
	int const  max_y = room.GetBox().inner_max(c_AxisY);

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
	int max_x = box.max(c_AxisX);
	int max_y = box.max(c_AxisY);
	for (int x = box.min.x; x < max_x; ++x)
	{
		for (int y = box.min.y; y < max_y; ++y)
		{
			Feature::spawn(Vec3{ x, y, map_z }, Terrain::Desk);
		}
	}
}

void MapGridder::add_unlocked_doors(int room_index) const
{
	Room const& room = m_generator.GetRoom(room_index);
	if (!room.IsCorridor())
	{
		DebugBreak("Only use MapGridder::add_unlocked_doors for corridors.");
	}

	Vec2 const door0 = room.GetBox().min;
	Vec2 const door1 = room.GetBox().inner_max();

	if (room.CorridorLength() >= 3)
	{
		// can be doors on both ends
		add_unlocked_door(door0);
		add_unlocked_door(door1);
	}
	else
	{
		// only one door
		// TODO: Only if both positions are good?

		if (Random::coinflip())
		{
			add_unlocked_door(door0);
		}
		else
		{
			add_unlocked_door(door1);
		}
	}
}

void MapGridder::add_unlocked_door(Vec2 const& pos) const
{
	// this door can be beside things as long as the position itself is good
	if (is_good_floor(pos))
	{
		Door::Unlocked door_type = m_doors.choose_unlocked();
		if (door_type != Door::Unlocked::None)
		{
			Terrain::Type door_terrain = Door::get_terrain(door_type);
			Feature::spawn(pos.xyz(m_map.get_z()), door_terrain);
		}
	}
}


//-----------------------------------------------------------------------------
// Pass 4 functions

void MapGridder::replace_all(Terrain::Type old_type, Terrain::Type new_type) const
{
	Box2 const& box = m_map.get_box();

	int const map_z = m_map.get_z();
	int max_x = box.max(c_AxisX);
	int max_y = box.max(c_AxisY);
	for (int x = box.min.x; x < max_x; ++x)
	{
		for (int y = box.min.y; y < max_y; ++y)
		{
			if (m_map.get_terrain(Vec2{ x, y }) == old_type)
			{
				m_map.set_terrain(Vec2{ x, y }, new_type);
			}
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
		if (room.GetBox().size.x % 2 == 0 &&  // size is even
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
		if (room.GetBox().size.y % 2 == 0 &&  // size is even
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
		// 4 torches in the corners
		Vec2 const pos_min = room.GetBox().min         + Vec2{ 1, 1 };
		Vec2 const pos_max = room.GetBox().inner_max() - Vec2{ 1, 1 };
		result.push_back(Vec2{ pos_min.x, pos_min.y });
		result.push_back(Vec2{ pos_min.x, pos_max.y });
		result.push_back(Vec2{ pos_max.x, pos_min.y });
		result.push_back(Vec2{ pos_max.x, pos_max.y });
	}
	else if (room.GetBox().size.x % 2 != 0 &&
	         room.GetBox().size.y >= 5 &&
	         room.GetBox().size.y > room.GetBox().size.x)
	{
		// 2 torches at the Y ends of the room
		int centre_x = room.GetBox().centre(c_AxisX);
		int min_y = room.GetBox().min.y + 1;
		int max_y = room.GetBox().inner_max(c_AxisY) - 1;
		result.push_back(Vec2{ centre_x, min_y });
		result.push_back(Vec2{ centre_x, max_y });
	}
	else if (room.GetBox().size.y % 2 != 0 &&
	         room.GetBox().size.x >= 5 &&
	         room.GetBox().size.x > room.GetBox().size.y)
	{
		// 2 torches at the X ends of the room
		int centre_y = room.GetBox().centre(c_AxisY);
		int min_x = room.GetBox().min.x + 1;
		int max_x = room.GetBox().inner_max(c_AxisX) - 1;
		result.push_back(Vec2{ min_x, centre_y });
		result.push_back(Vec2{ max_x, centre_y });
	}
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
	result.clear();

	// 1 torch along the wall at random
	PosTempList wall_positions = get_good_positions_along_wall(room);
	if (Util::Size(wall_positions) > 0)
	{
		result.push_back(Random::from_vector(wall_positions));
	}
	return result;
}

MapGridder::PosTempList MapGridder::get_good_positions_along_wall(Room const& room) const
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
	int const x_max = room.GetBox().inner_max(c_AxisX);
	int const y_max = room.GetBox().inner_max(c_AxisY);

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

MapGridder::PosTempList MapGridder::get_good_positions_inside_wall(Room const& room) const
{
	// Goal: Find all positions
	//  1. Inside the room wall
	//  2. With open space in front
	//  3. That are surrounded by wall on 5 sides, including 2 diagonally
	//
	// This has to be a separate function from get_good_positions_along_wall.
	//  -> The requirements are too different.

	PosTempList result_vec;

	// find room edges

	int const inside_x_min = room.GetBox().min.x;
	int const inside_y_min = room.GetBox().min.y;
	int const inside_x_max = room.GetBox().inner_max(c_AxisX);
	int const inside_y_max = room.GetBox().inner_max(c_AxisY);

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

MapGridder::PosTempList MapGridder::box_to_positions(Box2 const& box) const
{
	PosTempList result;

	int max_x = box.max(c_AxisX);
	int max_y = box.max(c_AxisY);
	for (int x = box.min.x; x < max_x; ++x)
	{
		for (int y = box.min.y; y < max_y; ++y)
		{
			result.push_back(Vec2{ x, y });
		}
	}
	return result;
}


//-----------------------------------------------------------------------------
// Functions to check if positions are good

bool MapGridder::is_good_for_spawn(Vec2 const& pos) const
{
	Terrain::Type terrain = m_map.get_terrain(pos);
	return terrain == Terrain::Wall || Terrain::can_spawn(terrain);
}

bool MapGridder::is_good_floor(Vec2 const & pos) const
{
	return Terrain::can_spawn(m_map.get_terrain(pos));
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
	int max_x = box.max(c_AxisX);
	int max_y = box.max(c_AxisY);
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
