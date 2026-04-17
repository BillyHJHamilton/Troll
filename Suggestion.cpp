#include "Suggestion.h"

#include "Types.h"
#include "Geometry.h"
#include "Serialize.h"
#include "VectorUtil.h"

#include <cassert>
#include <string>

namespace Suggestion
{

//-------------------------------------------------------------------------------------------------
// Global interface

bool is_valid_type(Suggestion::Type t)
{
	return t >= First && t < Count;
}

Genus get_genus(Type t)
{
	switch (t)
	{
	case Desk:
	case Torch:
		return Genus::Feature;
	case Bean:
	case TreasureNormal:
		return Genus::Item;
	case PlayerStart:
	case EnemyWeak:
	case EnemyModerate:
	case EnemyStrong:
		return Genus::Creature;
	default:
		return Genus::Unknown;
	}
}

Type get_enemy_type(float map_difficulty,
                    float enemy_difficulty)
{
	if (enemy_difficulty <= map_difficulty - 1.0f)
		return EnemyWeak;
	if (enemy_difficulty >= map_difficulty + 1.0f)
		return EnemyStrong;
	return EnemyModerate;
}

//-------------------------------------------------------------------------------------------------
// Suggestion Manager

Manager::Manager()
{
	int constexpr c_DefaultCapacity = 10;

	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		m_simple_vecs[i].reserve(c_DefaultCapacity);
	}

	m_secret_area_vec.reserve(c_DefaultCapacity);
	m_secret_passage_vec.reserve(c_DefaultCapacity);
}

void Manager::serialize(ISerializer & s)
{
	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		s.srz_vector(m_simple_vecs[i], "m_simple_vecs[i]");
		//s.srz_vector(m_simple_vecs[i], "m_simple_vecs[" + std::to_string(i) + "]");
	}

	s.srz_vector(m_secret_area_vec, "m_secret_area_vec");
	s.srz_vector(m_secret_passage_vec, "m_secret_passage_vec");
}

int Manager::get_total_count() const
{
	int count = 0;

	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		count += Util::Size(m_simple_vecs[i]);
	}

	count += Util::Size(m_secret_area_vec);
	count += Util::Size(m_secret_passage_vec);

	return count;
}

int Manager::get_count(Type type) const
{
	assert(is_valid_type(type));

	return Util::Size(m_simple_vecs[type]);
}

int Manager::get_count_secret_areas() const
{
	return Util::Size(m_secret_area_vec);
}

int Manager::get_count_secret_passages() const
{
	return Util::Size(m_secret_passage_vec);
}

SimpleList const & Manager::get(Type type) const
{
	return m_simple_vecs[type];
}

void Manager :: add_desk(Vec2 position)
{
	m_simple_vecs[Desk].push_back(position);
}

void Manager :: add_torch(Vec2 position)
{
	m_simple_vecs[Torch].push_back(position);
}

void Manager :: add_treasure_normal(Vec2 position)
{
	m_simple_vecs[TreasureNormal].push_back(position);
}

void Manager :: add_player_start(Vec2 position)
{
	m_simple_vecs[PlayerStart].push_back(position);
}

void Manager :: add_enemy_weak(Vec2 position)
{
	m_simple_vecs[EnemyWeak].push_back(position);
}

void Manager :: add_enemy_moderate(Vec2 position)
{
	m_simple_vecs[EnemyModerate].push_back(position);
}

void Manager :: add_enemy_strong(Vec2 position)
{
	m_simple_vecs[EnemyStrong].push_back(position);
}

void Manager :: add_boss(Vec2 position)
{
	m_simple_vecs[Boss].push_back(position);
}

void Manager :: add_secret_area(Vec2 door)
{
	SecretAreaInstance instance =
	{ .door = door,
	};
	m_secret_area_vec.push_back(instance);
}

void Manager :: add_secret_area(Vec2 door, Vec2 button, Vec2 torch)
{
	SecretAreaInstance instance =
	{ .door = door,
	  .button = button,
	  .torch1 = torch,
	  .trigger_types = TriggerTypes::ButtonOr1Torch,
	};
	m_secret_area_vec.push_back(instance);
}

void Manager :: add_secret_area(Vec2 door, Vec2 button,
                                Vec2 torch1, Vec2 torch2, Vec2 torch3, Vec2 torch4)
{
	SecretAreaInstance instance =
	{ .door = door,
	  .button = button,
	  .torch1 = torch1,
	  .torch2 = torch2,
	  .torch3 = torch3,
	  .torch4 = torch4,
	  .trigger_types = TriggerTypes::ButtonOr4Torches,
	};
	m_secret_area_vec.push_back(instance);
}
void Manager :: add_secret_passage(Vec2 door1, Vec2 door2)
{
	SecretPassageInstance instance =
	{ .door1 = door1,
	  .door2 = door2,
	};
	m_secret_passage_vec.push_back(instance);
}

void Manager :: add_secret_passage(Vec2 door1, Vec2 door2, Vec2 button1, Vec2 button2)
{
	SecretPassageInstance instance =
	{ .door1 = door1,
	  .door2 = door2,
	  .button1 = button1,
	  .button2 = button2,
	  .has_buttons = true,
	};
	m_secret_passage_vec.push_back(instance);
}

void Manager :: remove(Type type, int index)
{
	assert(is_valid_type(type));
	assert(index < get_count(type));

	Util::RemoveSwap(m_simple_vecs[type], index);
}

void Manager :: remove_secret_area(int index)
{
	assert(index < get_count_secret_areas());

	Util::RemoveSwap(m_secret_area_vec, index);
}

void Manager :: remove_secret_passage(int index)
{
	assert(index < get_count_secret_passages());

	Util::RemoveSwap(m_secret_passage_vec, index);
}

} // namespace Suggestion
