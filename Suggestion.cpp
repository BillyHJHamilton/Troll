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

Genus GetGenus(Type t)
{
	switch (t)
	{
	case SecretArea:
	case SecretPassage:
	case Pillar:
	case Desk:
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

//-------------------------------------------------------------------------------------------------
// Suggestion Manager

Manager::Manager()
{
	static int const c_DefaultCapacity = 10;

	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		m_Suggestions[i].reserve(c_DefaultCapacity);
	}
}

void Manager::serialize(ISerializer & s)
{
	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		s.srz_vector(m_Suggestions[i], "m_Suggestions[i]");
		//s.srz_vector(m_Suggestions[i], "m_Suggestions[" + std::to_string(i) + "]");
	}
}

int Manager::get_count(Type type) const
{
	assert(is_valid_type(type));

	return Util::Size(m_Suggestions[type]);
}

bool Manager :: has_any(Type type) const
{
	assert(is_valid_type(type));

	return !m_Suggestions[type].empty();
}

std::vector<Instance> const & Manager::get(Type type) const
{
	assert(is_valid_type(type));

	return m_Suggestions[type];
}


void Manager :: add_secret_area(Vec2 door)
{
	Instance instance =
	{ .position1 = door,
	  .is_button = false,
	};
	m_Suggestions[SecretArea].push_back(instance);
}

void Manager :: add_secret_area(Vec2 door, Vec2 button)
{
	Instance instance =
	{ .position1 = door,
	  .button1 = button,
	  .is_button = true,
	};
	m_Suggestions[SecretArea].push_back(instance);
}

void Manager :: add_secret_passage(Vec2 door1, Vec2 door2)
{
	Instance instance =
	{ .position1 = door1,
	  .position2 = door2,
	  .is_button = false,
	};
	m_Suggestions[SecretArea].push_back(instance);
}

void Manager :: add_secret_passage(Vec2 door1, Vec2 door2, Vec2 button1, Vec2 button2)
{
	Instance instance =
	{ .position1 = door1,
	  .position2 = door2,
	  .button1 = button1,
	  .button2 = button2,
	  .is_button = true,
	};
	m_Suggestions[SecretArea].push_back(instance);
}

void Manager :: add_treasure_normal(Vec2 position)
{
	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[TreasureNormal].push_back(instance);
}

void Manager :: add_player_start(Vec2 position)
{
	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[PlayerStart].push_back(instance);
}

void Manager :: add_enemy_weak(Vec2 position)
{
	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[EnemyWeak].push_back(instance);
}

void Manager :: add_enemy_moderate(Vec2 position)
{
	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[EnemyModerate].push_back(instance);
}
void Manager :: add_enemy_strong(Vec2 position)
{
	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[EnemyStrong].push_back(instance);
}

void Manager :: remove(Type type, int index)
{
	assert(is_valid_type(type));
	assert(index < get_count(type));

	Util::RemoveSwap(m_Suggestions[type], index);
}

} // namespace Suggestion
