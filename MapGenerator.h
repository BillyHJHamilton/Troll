#pragma once

#include "Types.h"
#include "Room.h"
#include "Stairs.h"

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

		int MaxCorridorLength = 6;

		void Serialize(ISerializer& s);
	};

	MapGenerator(Map& map);

	void Serialize(ISerializer& s);

	void SetParameters(Parameters parameters) { m_Param = parameters; }
	void RequestConnection(int targetMapId, int numConnections);

	// Generates rooms and tries to join everything up.
	void Generate();

	void AddTunnelTo(MapGenerator& other, int numToAdd);
	bool TryReceiveTunnel(Vec2 entry, Axis corridorAxis);

	void AddStairsTo(MapGenerator& other, int numToAdd);
	bool TryReceiveStairs(int sender_z, Stairs::Pair stairs_pair);

protected:
	using PosTempList = std::vector<Vec2,Scratch<Vec2>>;
	using Box2TempList = std::vector<Box2,Scratch<Box2>>;
	// should these be in VectorUtil.h?

	int FindRoomAtPos(Vec2 pos) const;

	// Map Gen Helpers
	void PlaceFirstRoomIfNeeded();
	void MarkExistingRoomsJoined();
	void PlaceRooms();
	void AddJoiningCorridors();
	void RemoveDisconnectedRooms();
	void AddExtraCorridors(int chance, bool isSecretPassages);
	void AssignRoomsToRegions();
	void AddAllToMap();

	// Map Gen Helper Helpers
	Vec2 RandRoomSize() const;
	Vec2 RandRoomPos(Vec2 roomSize) const;
	Stairs::Pair RandStairsPos(bool isUp) const;
	Room MakeRandomChamber() const;
	bool IsValidRoom(Room const &room, bool checkBorder) const;
	bool TryAddLandingRoom(int roomIndex); // This version only does rooms
	bool TryAddAdjoiningRoomForCorridor(int corridorRoomIndex, Vec2 joinEnd);
	void RemoveRoomFromAllNeighbourLists(int roomIndex);
	void RenumberRoomInAllNeighbourLists(int oldRoomIndex, int newRoomIndex);
	bool AreRoomsAlreadyConnected(int roomIndex1, int roomIndex2) const;
	void RemoveInvalidRoomsFromOptions(Room::TempList &options, bool check_borders);
	void RemoveBadlyPlacedStairsFromOptions(Room::TempList &options, Box2 otherMapBox);
	bool IsBadlyPlacedStairs(Room const& new_stairs, Box2 otherMapBox) const;
	bool AreStairsProblematic(Room const& new_stairs, Room const& other_stairs) const;
	void MakeRoomARegionParent(int roomIndex);

	void AddStairsToMap(Room const & room) const;
	void AddChamberToMap(Room const & room) const;
	void AddCorridorToMap(Room const & room) const;
	void AddDeskRoomSuggestions(Room const & room) const;
	void AddCosmeticTorchRoomSuggestions(Room const & room) const;
	void AddArmourRoomSuggestions(Room const & room) const;
	void AddSecretPassageSuggestions(Room const & room,
	                                 Room const & neighbour0, Vec2 const & door0,
	                                 Room const & neighbour1, Vec2 const & door1) const;
	void AddSecretAreaSuggestions(Room const & room,
	                              Room const & neighbour, Vec2 const & door) const;
	Vec2 GetPosAtRoomBack(Room const & room) const;
	PosTempList GetTorchPositions(Room const & room) const;
	PosTempList GetPositionsAlongPlainWall(Room const & room) const;
	PosTempList GetPlainWallPositions(Room const & room) const;
	static bool isContainedByAnyInList(Vec2 const & pos, Box2TempList const & boxVec);
	static bool isAnyContainedByAnyInList(PosTempList const & posVec,
	                                      Box2TempList const & boxVec);

	void PrintAllRooms() const;

	struct RequestedConnection
	{
		int target_level = 0;
		int num_to_add = 0;
	};
	std::vector<RequestedConnection> m_RequestedConnections;

	// Seed rooms are provided before Generate is run.  These are presumed to be valid.
	// Normally they will be added with AddStairsTo or AddTunnelTo.
	// If no seed rooms are added, we'll add a random chamber and make it a seed room.
	std::vector<Room> m_RoomVec;
	std::vector<int> m_JoinedRooms; // indices

	struct Region
	{
		int parent = Room::c_NoRegion;
		std::vector<int> rooms; // indices

		void serialize(ISerializer& s);
	};
	std::vector<Region> m_RegionVec;

	Map& m_Map;
	Parameters m_Param = {};
	bool m_HasGenerated = false;
};
