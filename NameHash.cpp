#include "NameHash.h"

#include <unordered_map>

//-----------------------------------------------------------------------------
// Map to check for name collisions on debug builds.
//-----------------------------------------------------------------------------

#if DEBUG_NAME_HASHING

namespace NameHashPrivate
{
	std::unordered_map<uint64_t, std::string>& CollisionMap()
	{
		static std::unordered_map<uint64_t, std::string> s_Instance;
		return s_Instance;
	}
}

#endif //CHECK_NAME_HASHING

//-----------------------------------------------------------------------------
// Class implementation
//-----------------------------------------------------------------------------

#if DEBUG_NAME_HASHING
// Check hashing in debug.
NameHash::NameHash(const char* const name) : m_Hash( NameHashPrivate::hash_64_fnv1a_const(name) )
{
	auto Itr = NameHashPrivate::CollisionMap().find(m_Hash);
	if (Itr == NameHashPrivate::CollisionMap().end())
	{
		NameHashPrivate::CollisionMap().emplace(m_Hash, name);
	}
	else
	{
		// Name collision!
		assert(Itr->second == name);
	}
}

std::string NameHash::GetNameString() const
{
	auto Itr = NameHashPrivate::CollisionMap().find(m_Hash);
	assert(Itr != NameHashPrivate::CollisionMap().end());
	return Itr->second;
}
#endif

#if UNIT_TESTS
void NameHash::UnitTest()
{
	NameHash Squash("Squash");
	NameHash Pumpkin("Pumpkin");
	NameHash Pumpkin2("Pumpkin");

	assert(Squash != Pumpkin);
	assert(Pumpkin == Pumpkin2);

	constexpr uint64_t Wow = NameHash::StaticHash("Wow");
	assert(Wow == NameHash("Wow"));
}
#endif
