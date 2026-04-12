#pragma once

#include "Types.h"
#include "Geometry.h"

namespace Suggestion
{
	// Use int because used as an array index

	enum Type : int
	{
		First = 0,

		// region with 1 entrance (placeholder)
		SecretArea = First,

		// region with 2 entrances (placeholder)
		//  -> both entrances will be locked the same way
		SecretPassage,

		Pillar,  // placeholder
		Desk,  // placeholder

		Bean,  // placeholder
		TreasureNormal,  // e.g. chest

		PlayerStart,
		EnemyWeak,      // below map difficulty
		EnemyModerate,  //  near map difficulty
		EnemyStrong,    // above map difficulty
		Count,
	};

	enum class Genus : byte
	{
		Feature = 0,
		Item,
		Creature,
		Unknown,
	};

	struct Instance
	{
		Vec2 position1 = {0, 0};
		Vec2 position2 = {0, 0};
		Vec2 button1 = {0, 0};
		Vec2 button2 = {0, 0};
		bool is_button = false;
	};

	Genus GetGenus(Suggestion::Type t);
	bool is_valid_type(Suggestion::Type t);

	Type get_enemy_type(float map_difficulty,
	                    float enemy_difficulty);

	class Manager
	{
	public:
		Manager();

		void serialize(ISerializer& s);

		int get_count(Type type) const;
		bool has_any(Type type) const;
		std::vector<Instance> const & get(Type type) const;

		void add_secret_area(Vec2 door);  // no secret areas yet
		void add_secret_area(Vec2 door, Vec2 button);
		void add_secret_passage(Vec2 door1, Vec2 door2);  // no secret passages yet
		void add_secret_passage(Vec2 door1, Vec2 door2, Vec2 button1, Vec2 button2);
		void add_treasure_normal(Vec2 position);
		void add_player_start(Vec2 position);
		void add_enemy_weak(Vec2 position);
		void add_enemy_moderate(Vec2 position);
		void add_enemy_strong(Vec2 position);
		// TODO: Add more functions when needed

		void remove(Type type, int index);

	private:
		std::vector<Instance> m_Suggestions[Type::Count];
	};
}
