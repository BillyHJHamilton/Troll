#pragma once

#include "Types.h"
#include "Geometry.h"
#include "Stairs.h"

enum class RoomType : byte
{
	Chamber,
	Corridor,
	Stairs
};

class Room
{
protected:
	// Constructors to produce different room types.  Use the named versions below.
	Room(Box2 box, RoomType roomType);
	Room(Box2 box, Axis corridorAxis); // corridor constructor
	Room(Vec2 localEnd, Stairs::Direction stairsDirection); // stairs constructor

public:
	inline static Room MakeChamber(Box2 box) { return Room(box, RoomType::Chamber); }
	inline static Room MakeCorridor(Box2 box, Axis corridorAxis) { return Room(box, corridorAxis); }
	inline static Room MakeStairs(Vec2 localEnd, Stairs::Direction direction) { return Room(localEnd, direction); }

	const Box2 &GetBox() const { return m_Box; }
	RoomType GetRoomType() const { return m_RoomType; }
	bool IsChamber() const { return m_RoomType == RoomType::Chamber; }
	bool IsCorridor() const { return m_RoomType == RoomType::Corridor; }
	bool IsStairs() const { return m_RoomType == RoomType::Stairs; }
	int CorridorLength() const;

	bool JoinsToRoom(Room const &room) const; // Stairs and corridors only
	bool VetoesRoom(Room const &newRoom) const;
	std::vector<Room> FindPossibleJoiningCorridors(Room const & other) const;
	Vec2 SuggestRandAdjoiningPositionForRoom(Vec2 roomSize) const;
	std::vector<Room> FindPossibleJoiningStairs(bool goingUp) const;

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
	Room FindPossibleJoiningCorridorCommon(Room const &other, Axis corridorAxis, int posOnOtherAxis) const;
	std::vector<Room> FindPossibleJoiningCorridorsAsStairs(Room const & other) const;

	Box2 m_Box; // Space occupied by the room, in (2D) global space
	RoomType m_RoomType;

	Axis m_CorridorAxis = AXIS_X;
	Stairs::Direction m_StairsDirection = Stairs::None;
};
