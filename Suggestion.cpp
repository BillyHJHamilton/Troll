#include "Suggestion.h"

#include "Types.h"
#include "Geometry.h"
#include "Serialize.h"
#include "VectorUtil.h"

#include <cassert>
#include <string>



Suggestion::Genus Suggestion::GetGenus(Suggestion::Type t)
{
	switch (t)
	{
	case SecretRegion:
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

int Suggestion::GetPositionCount(Suggestion::Type t)
{
	switch (t)
	{
	case SecretPassage:
		return 2;
	default:
		return 1;
	}
}

int Suggestion::GetSupportCount(Suggestion::Type t)
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

bool Suggestion::isWhen(Suggestion::Type t)
{
	switch (t)
	{
	case PlayerStart:
		return false;
	default:
		return GetGenus(t) == Genus::Creature;
	}
}



Suggestion::Manager::Manager()
{
	static int const c_DefaultCapacity = 0;

	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		m_Suggestions[i].reserve(c_DefaultCapacity);
	}
}

void Suggestion::Manager::serialize(ISerializer & s)
{
	static_assert(Type::First == 0, "Suggestion::Type::First must be 0");
	for (int i = Type::First; i < Type::Count; ++i)
	{
		s.srz_vector(m_Suggestions[i], "m_Suggestions[i]");
		//s.srz_vector(m_Suggestions[i], "m_Suggestions[" + std::to_string(i) + "]");
	}
}

int Suggestion::Manager::GetCount(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return Util::Size(m_Suggestions[type]);
}

bool Suggestion :: Manager :: isAny(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return !m_Suggestions[type].empty();
}

std::vector<Suggestion::Instance> const & Suggestion::Manager::getByType(Type type) const
{
	assert(type >= Type::First);
	assert(type <  Type::Count);

	return m_Suggestions[type];
}


// add map suggestions
//  -> only one function is valid for each type

void Suggestion :: Manager :: Add(Type type, Vec2 position)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 0);
	assert(isWhen(type)           == false);

	Instance instance =
	{ .position1 = position,
	};
	m_Suggestions[type].push_back(instance);
}

void Suggestion :: Manager :: Add(Type type, Vec2 position, Vec2 support)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 1);
	assert(isWhen(type)           == false);

	Instance instance =
	{ .position1 = position,
	  .support1 = support,
	};
	m_Suggestions[type].push_back(instance);
}

void Suggestion :: Manager :: Add(Type type, Vec2 position1, Vec2 position2, Vec2 support1, Vec2 support2)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 2);
	assert(GetSupportCount(type)  == 2);
	assert(isWhen(type)           == false);

	Instance instance =
	{ .position1 = position1,
	  .position2 = position2,
	  .support1 = support1,
	  .support2 = support2,
	};
	m_Suggestions[type].push_back(instance);
}

void Suggestion :: Manager :: Add(Type type, Vec2 position, When when)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(GetPositionCount(type) == 1);
	assert(GetSupportCount(type)  == 0);
	assert(isWhen(type)           == true);

	Instance instance =
	{ .position1 = position,
	  .when      = when,
	};
	m_Suggestions[type].push_back(instance);
}

void Suggestion :: Manager :: Remove(Type type, int index)
{
	assert(type >= Type::First);
	assert(type <  Type::Count);
	assert(index < GetCount(type));

	Util::RemoveSwap(m_Suggestions[type], index);
}
