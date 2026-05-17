#pragma once

#include "Types.h"
#include "Door.h"
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

		bool PlaceShopSeed = false;

		int MinAmbushRooms = 0;
		int MaxAmbushRooms = 2;

		// Fraction of cosmetic torches (no triggers) that start lit
		int percent_torches_lit = 50;
		int percent_monster_on_trigger = 20;

		void Serialize(ISerializer& s);
	};

	MapGenerator(Map& map);

	void Serialize(ISerializer& s);

	void SetParameters(Parameters parameters) { m_Param = parameters; }
	Parameters const & ReadParameters() const { return m_Param; }
	Parameters & EditParameters() { return m_Param; }

	void RequestConnection(int targetMapId, int numConnections);

	int GetRoomCount() const;
	Room const& GetRoom(int roomIndex) const { return m_RoomVec[roomIndex]; }
	std::vector<Room> const& GetRoomVector() const { return m_RoomVec; }
	bool IsStartRoom() const { return m_StartRoomIndex != c_Invalid; }
	bool IsStartRoom(int roomIndex) const { return roomIndex == m_StartRoomIndex; }
	bool IsRoomSecretPassage(int roomIndex) const
		{ return IsRegionSecretPassage(m_RoomVec[roomIndex].GetRegion()); }
	int GetRoomNeighbourCountExcludingSecretPassages(int roomIndex) const;

	int GetRegionCount() const;
	int GetRegionParent(int regionIndex) const { return m_RegionVec[regionIndex].parent; }
	bool IsRegionSecretPassage(int regionIndex) const
		{ return m_RegionVec[regionIndex].parent == Room::c_SecretPassage; }
	bool IsStartRegionOrAncestorOfIt(int regionIndex) const;

	// Generates rooms and tries to join everything up.
	void Generate();

	// Add connections from other maps
	void AddTunnelTo(MapGenerator& other, int numToAdd);
	bool TryReceiveTunnel(Vec2 entry, Axis corridorAxis);

	void AddStairsTo(MapGenerator& other, int numToAdd);
	bool TryReceiveStairs(int sender_z, Stairs::Pair stairs_pair);

protected:
	// Map Gen Helpers
	void PlaceFirstRoomIfNeeded();
	void MarkExistingRoomsJoined();
	void PlaceRooms();
	void AddJoiningCorridors();
	void RemoveDisconnectedRooms();
	void AddExtraCorridors(int chance, bool isSecretPassages);
	void AssignRoomsToRegions();

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

	// Debugging
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
	int m_StartRoomIndex = c_Invalid;
};
