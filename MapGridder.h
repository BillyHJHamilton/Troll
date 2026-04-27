#pragma once

#include "Types.h"
#include "Geometry.h"
#include "Scratch.h"

// This class converts the Rooms-based graph representation of a map
//   to the Terrain-based grid representation.  It also creates
//   Features and Suggestions for the map.
// After a MapGridder has been constructed, it can be destroyed.
class MapGridder
{
public:
	MapGridder(Map& map, MapGenerator& generator, int map_id);

protected:
	using PosTempList = std::vector<Vec2,Scratch<Vec2>>;
	using Box2TempList = std::vector<Box2,Scratch<Box2>>;
	// should these be in VectorUtil.h?

	// Pass 1 functions
	void add_basic(int room_index) const;

	// Pass 2 functions
	void add_treasure_suggestions() const;
	void add_shop_seed() const;

	void add_corridor_doors(int room_index) const;
	void add_secret_area(Room const & room,
	                     Room const & neighbour, Vec2 const & door) const;
	void add_secret_passage(Room const & room,
	                        Room const & neighbour0, Vec2 const & door0,
	                        Room const & neighbour1, Vec2 const & door1) const;

	TriggerType choose_trigger_type(bool allow_none, bool allow_button, bool allow_torch) const;
	LockedDoorType choose_locked_door_type(TriggerType trigger_type, bool allow_none) const;
	static Terrain::Type get_terrain_for_locked_door_type(LockedDoorType door_type);
	static Terrain::Type get_terrain_for_unlocked_door_type(UnlockedDoorType door_type);

	// Pass 3 functions
	void add_cosmetic_chamber(int room_index) const;
	void add_cosmetic_torches(Room const & room) const;
	void add_cosmetic_armour(Room const & room) const;
	void add_cosmetic_desks(Room const & room) const;
	void add_desks_in_box(Box2 const & box) const;

	void add_unlocked_doors(int room_index) const;

	// Pass 4 functions
	void replace_all(Terrain::Type old_type, Terrain::Type new_type) const;

	// Functions to select positions
	Vec2 get_pos_at_room_back(Room const & room) const;
	PosTempList choose_torch_positions(Room const & room) const;
	PosTempList get_good_positions_along_wall(Room const & room) const;
	PosTempList get_good_positions_inside_wall(Room const & room) const;
	PosTempList box_to_positions(Box2 const & box) const;

	// Functions to check if positions are good
	bool is_good_for_spawn(Vec2 const & pos) const;
	bool is_good_neighbours(Vec2 const & pos) const;
	bool is_good_floor(Vec2 const & pos) const;
	bool is_good_wall(Vec2 const & pos) const;
	bool is_good_for_isolated_floor(Vec2 const & pos) const;
	bool is_good_for_isolated_wall(Vec2 const & pos) const;
	bool is_good_for_isolated_floor(PosTempList const & pos_list) const;
	bool is_good_floor(Box2 const & box) const;

	bool is_inside_east_west_wall(Vec2 const & pos) const;
	bool is_inside_north_south_wall(Vec2 const & pos) const;
	bool is_by_east_wall(Vec2 const & pos) const
	{	return is_inside_north_south_wall(Vec2{ pos.x + 1, pos.y });	}
	bool is_by_north_wall(Vec2 const & pos) const
	{	return is_inside_east_west_wall(Vec2{ pos.x, pos.y - 1 });	}
	bool is_by_west_wall(Vec2 const & pos) const
	{	return is_inside_north_south_wall(Vec2{ pos.x - 1, pos.y });	}
	bool is_by_south_wall(Vec2 const & pos) const
	{	return is_inside_east_west_wall(Vec2{ pos.x, pos.y + 1 });	}

private:
	Map& m_map;
	const MapGenerator& m_generator;
};
