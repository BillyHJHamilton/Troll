#pragma once

#include <cstdint>
#include <unordered_map> // for std::hash template

//-----------------------------------------------------------------------------
// Hash function (external code)
//-----------------------------------------------------------------------------
 
// Original Source: https://gist.github.com/ruby0x1/81308642d0325fd386237cfa3b44785c
// Information: https://notes.underscorediscovery.com/constexpr-fnv1a/
// code license: "public domain or equivalent", so we can use freely

// FNV1a c++11 constexpr compile time hash functions, 32 and 64 bit
// str should be a null terminated string literal, value should be left out 
// e.g hash_32_fnv1a_const("example")

namespace NameHashPrivate
{
	constexpr uint32_t val_32_const = 0x811c9dc5;
	constexpr uint32_t prime_32_const = 0x1000193;
	constexpr uint64_t val_64_const = 0xcbf29ce484222325;
	constexpr uint64_t prime_64_const = 0x100000001b3;

	inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept {
		return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
	}

	inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
		return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
	}
}
// End of external code

class NameHash
{
public:
#if DEBUG_NAME_HASHING
	NameHash(const char* const name);
	std::string GetNameString() const;
#else
	constexpr NameHash(const char* const name) : m_Hash( NameHashPrivate::hash_64_fnv1a_const(name) ) {}
#endif
	explicit constexpr NameHash(uint64_t hash) : m_Hash(hash) {}
	static constexpr uint64_t StaticHash(const char* const name) { return NameHashPrivate::hash_64_fnv1a_const(name); }
	operator uint64_t() const { return m_Hash; }
	NameHash& operator=(NameHash other) { m_Hash = other.m_Hash; return *this; }
	bool operator==(NameHash other) const { return m_Hash == other.m_Hash; }
	bool operator!=(NameHash other) const { return !operator==(other); }
	bool operator<(NameHash other) const { return m_Hash < other.m_Hash; }
	bool IsValid() const { return m_Hash != None(); }
	static NameHash None() { return NameHash(""); }

#if UNIT_TESTS
	static void UnitTest();
#endif
private:
	uint64_t m_Hash;
};

// Support for unordered_map<NameHash>
namespace std
{
	template <>
	struct hash<NameHash>
	{
		size_t operator()(const NameHash & x) const
		{
			return std::hash<uint64_t>()(static_cast<uint64_t>(x));
		}
	};
}
