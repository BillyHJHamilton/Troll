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

	struct SecretAreaInstance
	{
		Vec2 door = {0, 0};
		Vec2 button = {0, 0};
		bool has_button = false;
	};

	struct SecretPassageInstance
	{
		Vec2 door1 = {0, 0};
		Vec2 door2 = {0, 0};
		Vec2 button1 = {0, 0};
		Vec2 button2 = {0, 0};
		bool has_buttons = false;
	};

	bool is_valid_type(Suggestion::Type t);
	bool is_simple_type(Suggestion::Type t);
	Genus get_genus(Suggestion::Type t);

	Type get_enemy_type(float map_difficulty,
	                    float enemy_difficulty);

	using SimpleList = std::vector<Vec2>;
	using SecretAreaList = std::vector<SecretAreaInstance>;
	using SecretPassageList = std::vector<SecretPassageInstance>;

	class Manager
	{
	public:
		Manager();

		void serialize(ISerializer& s);

		int get_total_count() const;
		int get_count(Type type) const;
		bool has_any(Type type) const;
		SimpleList const & get(Type type) const;
		SecretAreaList const & get_secret_areas() const { return m_secret_area_vec; }
		SecretPassageList const & get_secret_passages() const { return m_secret_passage_vec; }

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
		// Suggestions types that just need a position are stored here.
		// Types that need more data have an empty vector here.
		SimpleList m_simple_vecs[Type::Count];

		// Suggestions types that need more data have their own vectors.
		SecretAreaList m_secret_area_vec;
		SecretPassageList m_secret_passage_vec;
	};
}
