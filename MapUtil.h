#pragma once

#include <unordered_map>

namespace Util
{
	// If key is not found, add one using the default constructor.
	template<typename KeyType, typename ValueType>
	ValueType& FindOrAdd(std::unordered_map<KeyType,ValueType>& map, const KeyType& key)
	{
		auto pair = map.try_emplace(key, ValueType());
		return pair.first->second;
	}

	template<typename KeyType, typename ValueType>
	ValueType* Find(std::unordered_map<KeyType,ValueType>& map, KeyType key)
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

	template<typename KeyType, typename ValueType>
	const ValueType* Find(const std::unordered_map<KeyType,ValueType>& map, KeyType key)
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