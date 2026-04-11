#include "MapSuggestion.h"

#include "Types.h"
#include "Geometry.h"
#include "Serialize.h"
#include "VectorUtil.h"

#include <cassert>
#include <string>



MapSuggestion::Genus MapSuggestion::GetGenus(MapSuggestion::Type t)
{
	switch (t)
	{
	case SecretRegion:
	case SecretPassage:
	case Pillar:
	case Desk:
		return Genus::Feature;
	case TreasureBean:
	case TreasureNormal:
		return Genus::Treasure;
	case PlayerStart:
	case EnemyWeak:
	case EnemyNormal:
	case EnemyStrong:
		return Genus::Creature;
	default:
		return Genus::Unknown;
	}
}

int MapSuggestion::GetPositionCount(MapSuggestion::Type t)
{
	switch (t)
	{
	case SecretPassage:
		return 2;
	default:
		return 1;
	}
}

int MapSuggestion::GetSupportCount(MapSuggestion::Type t)
{
	switch (t)
	{
	case SecretRegion:
		return 1;
	case SecretPassage:
		return 2;
	default:
		return 0;
	}
}

bool MapSuggestion::isWhenToSpawn(MapSuggestion::Type t)
{
	switch (t)
	{
	case PlayerStart:
		return false;
	default:
		return GetGenus(t) == Genus::Creature;
	}
}



MapSuggestion::Manager::Manager()
{
	static int const c_DefaultCapacity = 0;

	static_assert(Type::First == 0, "MapSuggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		m_Suggestions[i].reserve(c_DefaultCapacity);
	}
}

void MapSuggestion::Manager::serialize(ISerializer & s)
{
	static_assert(Type::First == 0, "MapSuggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		s.srz_vector(m_Suggestions[i], "m_Suggestions[i]");
		//s.srz_vector(m_Suggestions[i], "m_Suggestions[" + std::to_string(i) + "]");
	}
}

int MapSuggestion::Manager::GetCount(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return Util::Size(m_Suggestions[type]);
}

bool MapSuggestion :: Manager :: isAny(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return !m_Suggestions[type].empty();
}

std::vector<MapSuggestion::Instance> const & MapSuggestion::Manager::getByType(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return m_Suggestions[type];
}


// add map suggestions
//  -> only one function is valid for each type

void MapSuggestion :: Manager :: Add(Type type, Vec2 position)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 0);
	assert(isWhenToSpawn(type)    == false);

	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[type].push_back(instance);
}

void MapSuggestion :: Manager :: Add(Type type, Vec2 position, Vec2 support)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 1);
	assert(isWhenToSpawn(type)    == false);

	Instance instance =
	{ .position1 = position,
	  .support1 = support,
	};
	m_Suggestions[type].push_back(instance);
}

void MapSuggestion :: Manager :: Add(Type type, Vec2 position1, Vec2 position2, Vec2 support1, Vec2 support2)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 2);
	assert(GetSupportCount(type)  == 2);
	assert(isWhenToSpawn(type)    == false);

	Instance instance =
	{ .position1 = position1,
	  .position2 = position2,
	  .support1 = support1,
	  .support2 = support2,
	};
	m_Suggestions[type].push_back(instance);
}

void MapSuggestion :: Manager :: Add(Type type, Vec2 position, WhenToSpawn when)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 0);
	assert(isWhenToSpawn(type)    == true);

	Instance instance =
	{ .position1 = position,
	  .when      = when,
	};
	m_Suggestions[type].push_back(instance);
}

void MapSuggestion :: Manager :: Remove(Type type, int index)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(index < GetCount(type));

	Util::RemoveSwap(m_Suggestions[type], index);
}
