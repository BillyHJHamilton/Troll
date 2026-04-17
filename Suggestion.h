#pragma once

#include "Types.h"
#include "Geometry.h"

namespace Suggestion
{
	// Use int because used as an array index

	enum Type : int
	{
		First = 0,

		Pillar,  // placeholder
		Desk,  // placeholder

		Bean,  // placeholder
		TreasureNormal,  // e.g. chest

		PlayerStart,
		EnemyWeak,      // below map difficulty
		EnemyModerate,  //  near map difficulty
		EnemyStrong,    // above map difficulty
		Boss,	// possible boss starting positions

		Count,
	};
	// secret areas and secret passages are handles separately

	enum class Genus : byte
	{
		Feature = 0,
		Item,
		Creature,
		Unknown,
	};

	bool is_valid_type(Suggestion::Type t);
	Genus get_genus(Suggestion::Type t);

	Type get_enemy_type(float map_difficulty,
	                    float enemy_difficulty);

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
		int get_count_secret_areas() const;
		int get_count_secret_passages() const;
		SimpleList const & get(Type type) const;
		SecretAreaList const & get_secret_areas() const { return m_secret_area_vec; }
		SecretPassageList const & get_secret_passages() const { return m_secret_passage_vec; }

		void add_treasure_normal(Vec2 position);
		void add_player_start(Vec2 position);
		void add_enemy_weak(Vec2 position);
		void add_enemy_moderate(Vec2 position);
		void add_enemy_strong(Vec2 position);
		void add_boss(Vec2 position);
		// TODO: Add more functions when needed

		void add_secret_area(Vec2 door);
		void add_secret_area(Vec2 door, Vec2 button);
		void add_secret_passage(Vec2 door1, Vec2 door2);
		void add_secret_passage(Vec2 door1, Vec2 door2, Vec2 button1, Vec2 button2);

		void remove(Type type, int index);
		void remove_secret_area(int index);
		void remove_secret_passage(int index);

	private:
		// Suggestions types that just need a position.
		SimpleList m_simple_vecs[Type::Count];

		// Suggestions types that need more data have their own vectors.
		SecretAreaList m_secret_area_vec;
		SecretPassageList m_secret_passage_vec;
	};
}
