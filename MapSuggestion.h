#pragma once

#include "Types.h"
#include "Geometry.h"

namespace MapSuggestion
{
	// Use int because used as an array index

	enum Type : int
	{
		First = 0,
		SecretRegion = First,  // 1 entrance
		SecretPassage,  // 2 entrances
		Pillar,
		Desk,
		TreasureBean,
		TreasureNormal,  // e.g. chest
		PlayerStart,
		EnemyWeak,  // below map difficulty
		EnemyNormal,  // map difficulty
		EnemyStrong,  // above map difficulty
		Count,
	};

	enum class Genus : byte
	{
		Feature = 0,
		Treasure,
		Creature,
		Unknown,
	};

	enum class WhenToSpawn : byte  // for creatures
	{
		Initial = 0,  // first spawns for map
		Later,  // once during later spawns
		//Repeating,  // always available - for vaults
	};

	struct Instance
	{
		Vec2 position1 = {0,0};
		Vec2 position2 = {0,0};
		Vec2 support1 = {0,0};
		Vec2 support2 = {0,0};
		WhenToSpawn when = WhenToSpawn::Initial;
	};

	Genus GetGenus(MapSuggestion::Type t);
	int GetPositionCount(MapSuggestion::Type t);
	int GetSupportCount(MapSuggestion::Type t);
	bool isWhenToSpawn(MapSuggestion::Type t);

	class Manager
	{
	public:
		Manager();

		void serialize(ISerializer& s);

		int GetCount(Type type) const;
		bool isAny(Type type) const;
		std::vector<Instance> const & getByType(Type type) const;

		// add map suggestions
		//  -> only one function is valid for each suggestion type
		void Add(Type type, Vec2 position);
		void Add(Type type, Vec2 position, Vec2 support);
		void Add(Type type, Vec2 position1, Vec2 position2, Vec2 support1, Vec2 support2);
		void Add(Type type, Vec2 position, WhenToSpawn when);

		void Remove(Type type, int index);

	private:
		std::vector<Instance> m_Suggestions[Type::Count];
	};
}
