#pragma once

#include "Types.h"
#include "Geometry.h"

namespace Suggestion
{
	// Use int because used as an array index
	enum Type : int
	{
		PlayerStart,
		EnemyWeak,      // below map difficulty
		EnemyModerate,  //  near map difficulty
		EnemyStrong,    // above map difficulty
		Boss,	// possible boss starting positions

		Count,
	};
	// treasures are handled separately

	bool is_valid_type(Suggestion::Type t);

	Type get_enemy_type(float map_difficulty,
	                    float enemy_difficulty);

	struct Treasure
	{
		Vec2 treasure_pos = {0, 0};
		Vec2 spawn_at = {0, 0};
		bool can_spawn = false;
	};

	using SimpleList = std::vector<Vec2>;
	using TreasureList = std::vector<Treasure>;

	class Manager
	{
	public:
		Manager();

		void serialize(ISerializer& s);

		int get_total_count() const;
		int get_count(Type type) const;
		int get_count_treasure() const;
		SimpleList const & get(Type type) const;
		TreasureList const & get_treasure() const;

		void add_player_start(Vec2 position);
		void add_enemy_weak(Vec2 position);
		void add_enemy_moderate(Vec2 position);
		void add_enemy_strong(Vec2 position);
		void add_boss(Vec2 position);

		void add_treasure(Vec2 position);
		void add_treasure(Vec2 position, Vec2 spawn_at);

		void remove(Type type, int index);
		void remove_treasure(int index);

	private:
		// Suggestions types that just need a position.
		SimpleList m_simple_vecs[Type::Count];

		// Suggestions types that need more data have their own vectors.
		TreasureList m_treasure_vec;
	};
}
