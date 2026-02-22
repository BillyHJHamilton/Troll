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
	Stairs
};

class Room
{
public:
	using TempList = std::vector<Room,Scratch<Room>>;

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

	void Serialize(ISerializer& s);

	const Box2 &GetBox() const { return m_Box; }
	RoomType GetRoomType() const { return m_RoomType; }
	bool IsChamber() const { return m_RoomType == RoomType::Chamber; }
	bool IsCorridor() const { return m_RoomType == RoomType::Corridor; }
	bool IsStairs() const { return m_RoomType == RoomType::Stairs; }
	Axis CorridorAxis() const;
	int CorridorLength() const;

	bool JoinsToRoom(Room const &room) const; // Stairs and corridors only
	bool VetoesRoom(Room const &newRoom) const;
	Room::TempList FindPossibleJoiningCorridors(Room const & other) const;
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
};
