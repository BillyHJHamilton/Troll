#pragma once

#include "Types.h"
#include "Geometry.h"
#include "Scratch.h"
#include "Stairs.h"

enum class RoomType : int
{
	Invalid = c_Invalid,
	Chamber,
	Corridor,
	Stairs,
	IntermapCorridor,
};

class Room
{
public:
	using TempList = std::vector<Room,Scratch<Room>>;

	static int constexpr c_MainRegion = 0;

protected:
	// Constructors to produce different room types.  Use the named versions below.
	Room(Box2 box, RoomType roomType);
	Room(Box2 box, Axis corridorAxis); // corridor constructor
	Room(Vec2 localEnd, Stairs::Direction stairsDirection); // stairs constructor

public:
	// Default constructor needed for serialization.  Makes invalid room.
	Room() : m_RoomType(RoomType::Invalid) {}

	inline static Room MakeChamber(Box2 box) { return Room(box, RoomType::Chamber); }
	inline static Room MakeCorridor(Box2 box, Axis corridorAxis) { return Room(box, corridorAxis); }
	inline static Room MakeStairs(Vec2 localEnd, Stairs::Direction direction) { return Room(localEnd, direction); }

	void serialize(ISerializer& s);

	const Box2 &GetBox() const { return m_Box; }
	RoomType GetRoomType() const { return m_RoomType; }
	bool IsChamber() const { return m_RoomType == RoomType::Chamber; }
	bool IsCorridor() const { return m_RoomType == RoomType::Corridor || m_RoomType == RoomType::IntermapCorridor; }
	bool IsStairs() const { return m_RoomType == RoomType::Stairs; }
	bool IsMapConnector() const { return m_RoomType == RoomType::Stairs || m_RoomType == RoomType::IntermapCorridor; }
	Axis CorridorAxis() const;
	int CorridorLength() const;
	void markCorridorAsMapConnector();

	bool JoinsToRoom(Room const &room) const; // Stairs and corridors only
	bool VetoesRoom(Room const &newRoom) const;
	Room::TempList FindPossibleJoiningCorridors(Room const &other) const;
	Room::TempList FindPossibleJoiningCorridorsToBox(Box2 other_box) const;
	Vec2 AsStairsSuggestRandAdjoiningPositionForRoom(Vec2 roomSize) const;
	Vec2 AsCorridorSuggestRandAdjoiningPositionForRoom(Vec2 roomSize, Vec2 joinEnd) const;
	Room::TempList FindPossibleJoiningStairs(bool goingUp) const;

	bool AnyRoomVetoes(const std::vector<Room> &roomVec) const;

	void AddToMap(Map &map) const;

	// stairs stuff
	Vec2 StairsLocalEnd() const;
	Vec2 StairsRemoteEnd() const;
	Stairs::Direction GetStairsDirection() const { return m_StairsDirection; }
	static Axis StairsAxis(Stairs::Direction direction);
	//static bool StairsGoUp(Stairs::Direction direction);
	//static Stairs::Direction StairsCorrespondingDirection(Stairs::Direction direction);

	// neighbours stuff
	std::vector<int> const& GetNeighbours() const { return m_Neighbours; }
	int GetNeighbourCount() const { return Util::Size(m_Neighbours); }
	bool IsNeighbourOf(int neighbour) const;
	void AddNeighbour(int neighbour);
	void RemoveNeighbour(int neighbour);
	void RenumberNeighbour(int oldNeighbour, int newNeighbour);

	int GetRegion() const { return m_Region; }
	bool IsInMainRegion() const { return m_Region == c_MainRegion; }
	void SetRegion(int region) { m_Region = region; }

private:
	// polymorphic stuff
	bool JoinsToRoomAsCorridor(Room const &room) const;
	bool JoinsToRoomAsStairs(Room const &room) const;
	Room FindPossibleJoiningCorridorCommon(Box2 other_box, Axis corridorAxis, int posOnOtherAxis) const;
	Vec2 SuggestRandAdjoiningPositionForRoomCommon(Vec2 roomSize, Vec2 joinEnd, Axis axis, int dir) const;
	Room::TempList FindPossibleJoiningCorridorsAsStairs(Box2 other_box) const;

	Box2 m_Box; // Space occupied by the room, in (2D) global space
	RoomType m_RoomType;

	Stairs::Direction m_StairsDirection = Stairs::None;
	Axis m_CorridorAxis = AXIS_X;

	int m_Region = c_MainRegion;
	std::vector<int> m_Neighbours; // indices of other rooms
};
