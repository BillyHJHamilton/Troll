#pragma once

#include "Types.h"
#include "Geometry.h"

namespace Suggestion
{
	// Use int because used as an array index
	enum Type : int
	{
		TreasureNormal,  // e.g. chest

		PlayerStart,
		EnemyWeak,      // below map difficulty
		EnemyModerate,  //  near map difficulty
		EnemyStrong,    // above map difficulty
		Boss,	// possible boss starting positions

		Count,
	};
	// secret areas and secret passages are handled separately

	bool is_valid_type(Suggestion::Type t);

	Type get_enemy_type(float map_difficulty,
	                    float enemy_difficulty);

	using SimpleList = std::vector<Vec2>;

	class Manager
	{
	public:
		Manager();

		void serialize(ISerializer& s);

		int get_total_count() const;
		int get_count(Type type) const;
		SimpleList const & get(Type type) const;

		void add_treasure_normal(Vec2 position);
		void add_player_start(Vec2 position);
		void add_enemy_weak(Vec2 position);
		void add_enemy_moderate(Vec2 position);
		void add_enemy_strong(Vec2 position);
		void add_boss(Vec2 position);

		void remove(Type type, int index);

	private:
		// Suggestions types that just need a position.
		SimpleList m_simple_vecs[Type::Count];
	};
}
