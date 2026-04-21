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
	return t >= 0 && t < Count;
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

	for (int i = 0; i < Type::Count; ++i)
	{
		m_simple_vecs[i].reserve(c_DefaultCapacity);
	}
}

void Manager::serialize(ISerializer & s)
{
	for (int i = 0; i < Type::Count; ++i)
	{
		s.srz_vector(m_simple_vecs[i], "m_simple_vecs[i]");
		//s.srz_vector(m_simple_vecs[i], "m_simple_vecs[" + std::to_string(i) + "]");
	}
}

int Manager::get_total_count() const
{
	int count = 0;

	for (int i = 0; i < Type::Count; ++i)
	{
		count += Util::Size(m_simple_vecs[i]);
	}

	return count;
}

int Manager::get_count(Type type) const
{
	assert(is_valid_type(type));

	return Util::Size(m_simple_vecs[type]);
}

SimpleList const & Manager::get(Type type) const
{
	return m_simple_vecs[type];
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

void Manager :: remove(Type type, int index)
{
	assert(is_valid_type(type));
	assert(index < get_count(type));

	Util::RemoveSwap(m_simple_vecs[type], index);
}

} // namespace Suggestion
