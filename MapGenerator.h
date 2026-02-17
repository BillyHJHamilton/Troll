#pragma once

#include "Types.h"
#include "Room.h"
#include "Stairs.h"

// NEXT STEPS:
// - Allow specifying another type of "seed" room, not just stairs.
// - Allow importing seed stairs from either above/below levels.

// The architecture is that each map owns its own "MapGenerator".
// The generator contains metadata like where rooms are located,
// whereas the Map layer exposes the resulting terrain and staircase table.
class MapGenerator
{
public:
	struct Parameters
	{
		int MapBorder = 1;

		int MinRoomDimension = 3;
		int MaxRoomDimension = 7;
		int MinRoomArea = 3*3;
		int MaxRoomArea = 6*6;

		int MinNumRooms = 8;
		int MaxNumRooms = 20;

		int MinStairsProximity = 4;
		int MinFacingStairsProximity = 6; // 3 + min room dimension

		int UpStairsToAdd = 2;
		int DownStairsToAdd = 0;

		void Serialize(ISerializer& s);
	};

	MapGenerator(Map& map);

	void Serialize(ISerializer& s);

	void SetParameters(Parameters parameters) { m_Param = parameters; }

	void AddConnectingStairsAsSeedRooms(Map const& map);

	void Generate();

	const std::vector<Stairs::Pair>& GetFailedStairs() const { return m_FailedStairs; }

private:
	int FindRoomAtPos(Vec2 pos);

	// Map Gen Helpers
	void PlaceSeedRooms();
	void PlaceRooms();
	void AddJoiningCorridors();
	void RemoveDisconnectedRooms();
	void AddExtraStairs(bool goingUp, int stairsToAdd);
	void AddExtraCorridors();

	// Map Gen Helper Helpers
	Vec2 RandRoomSize();
	Vec2 RandRoomPos(Vec2 roomSize);
	Stairs::Pair RandStairsPos(bool isUp);
	Room MakeRandomChamber();
	bool IsValidRoom(Room const &room, bool checkBorder);
	bool TryAddLanding(Room const &stairsRoom, int& chambersAdded, int& corridorsAdded);
	void RemoveInavlidRoomsFromOptions(std::vector<Room> &options);
	void RemoveBadlyPlacedStairsFromOptions(std::vector<Room> &options);
	bool IsBadlyPlacedStairs(Room const& new_stairs);
	bool AreStairsProblematic(Room const& new_stairs, Room const& other_stairs);

	// Seed rooms are provided before Generate is run.  These are presumed to be valid.
	// Normally they will be added with AddConnectingStaircases.
	// If no seed rooms are added, we'll add a random chamber and make it a seed room.
	std::vector<Room> m_SeedRooms;
	std::vector<Room> m_RoomVec;
	std::vector<int> m_JoinedRooms; // indices

	// Unfortunately sometimes the generator fails to place some of the requested seeds.
	// In that case we could redo the generation completely, or we could post-process
	// the previous level to remove the detached staircase.
	std::vector<Stairs::Pair> m_FailedStairs;

	Map& m_Map;
	Parameters m_Param = {};
	bool m_HasGenerated = false;
};
