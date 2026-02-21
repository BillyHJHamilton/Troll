#pragma once

#include <unordered_map>

namespace Util
{
	// Get the size and convert to int (to suppress size_t warnings).
	template<typename KeyType, typename ValueType, typename Hash, typename Keq, typename Alc>
	inline int Size(std::unordered_map<KeyType,ValueType,Hash,Keq,Alc> const& map)
	{
		return static_cast<int>(map.size());
	}

	// If key is not found, add one using the default constructor.
	template<typename KeyType, typename ValueType, typename Hash, typename Keq, typename Alc>
	ValueType& FindOrAdd(std::unordered_map<KeyType,ValueType,Hash,Keq,Alc>& map, const KeyType& key)
	{
		auto pair = map.try_emplace(key, ValueType());
		return pair.first->second;
	}

	template<typename KeyType, typename ValueType, typename Hash, typename Keq, typename Alc>
	ValueType* Find(std::unordered_map<KeyType,ValueType,Hash,Keq,Alc>& map, KeyType key)
	{
		auto itr = map.find(key);
		if (itr != map.end())
		{
			return &(itr->second);
		}
		else
		{
			return nullptr;
		}
	}

	template<typename KeyType, typename ValueType, typename Hash, typename Keq, typename Alc>
	const ValueType* Find(const std::unordered_map<KeyType,ValueType,Hash,Keq,Alc>& map, KeyType key)
	{
		auto itr = map.find(key);
		if (itr != map.end())
		{
			return &(itr->second);
		}
		else
		{
			return nullptr;
		}
	}
}