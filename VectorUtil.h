#pragma once

#include "Scratch.h"
#include "Types.h"

#include <algorithm>
#include <memory>
#include <vector>

using IntTempList = std::vector<int,Scratch<int>>;
using FloatTempList = std::vector<float,Scratch<float>>;

namespace Util
{
	// Get the size and convert to int (to suppress size_t warnings).
	template<class VectorItemType, typename Alc>
	inline int Size(std::vector<VectorItemType,Alc> const& vector)
	{
		return static_cast<int>(vector.size());
	}

	// Get index of the last item added to the back of the vector.
	template<class VectorItemType, typename Alc>
	inline int LastIndex(std::vector<VectorItemType,Alc> const& vector)
	{
		return Size(vector) - 1;
	}

	template<typename VectorItemType, typename Alc>
	bool IsValidIndex(std::vector<VectorItemType,Alc> const& vector, int index)
	{
		return index >= 0 && index < vector.size();
	}

	// Basic functions for searching.

	template<typename VectorItemType, typename Alc, typename ValueType>
	std::vector<VectorItemType,Alc>::const_iterator Find(std::vector<VectorItemType,Alc> const& vector, ValueType value)
	{
		return std::find(vector.cbegin(), vector.cend(), value);
	}

	// Non-const version which is needed for RemoveSwapFirstMatchingItem
	template<typename VectorItemType, typename Alc, typename ValueType>
	std::vector<VectorItemType,Alc>::iterator Find(std::vector<VectorItemType,Alc>& vector, ValueType value)
	{
		return std::find(vector.begin(), vector.end(), value);
	}

	template<typename VectorItemType, typename Alc, typename ValueType>
	int FindIndex(std::vector<VectorItemType,Alc> const& vector, ValueType value)
	{
		auto itr = std::find(vector.cbegin(), vector.cend(), value);
		if (itr == vector.cend())
		{
			return c_Invalid; // invalid
		}
		else
		{
			return static_cast<int>(itr - vector.cbegin());
		}
	}
	
	// Search for an entry by the value of one member (the key), using "pointer to member" syntax.
	// For example, to search for a Foo where .id==2, provide &Foo::id as the key_variable.
	template<typename VectorItemType, typename Alc, typename ValueType, typename KeyType>
	int FindIndexByKey(std::vector<VectorItemType,Alc> const& vector,
		KeyType ValueType::* key_variable, KeyType key_to_find)
	{
		for (int i = 0; i < Util::Size(vector); ++i)
		{
			if (vector[i].*key_variable == key_to_find)
			{
				return i;
			}
		}
		return c_Invalid; // key not found
	}

	template<typename VectorItemType, typename Alc, typename ValueType>
	bool Contains(std::vector<VectorItemType,Alc> const& vector, ValueType& value)
	{
		auto itr = std::find(vector.begin(), vector.end(), value);
		return itr != vector.end();
	}

	// Utility functions for removing with simpler syntax

	template<typename VectorItemType, typename Alc>
	void RemoveAt(std::vector<VectorItemType,Alc>& vector, int indexToRemove)
	{
		vector.erase(vector.begin() + indexToRemove);
	}

	template<typename VectorItemType, typename Alc, typename ValueType>
	void RemoveFirstMatchingItem(std::vector<VectorItemType,Alc>& vector, ValueType& value)
	{
		auto itr = Find(vector, value);
		if (itr != vector.end())
		{
			vector.erase(itr);
		}
	}
	
	template<typename VectorItemType, typename Alc, typename ValueType>
	void RemoveAllMatchingItems(std::vector<VectorItemType,Alc>& vector, ValueType& value)
	{
		for (int i = 0; i < vector.size(); ++i)
		{
			if (vector[i] == value)
			{
				RemoveAt(vector, i);
				--i;
			}
		}
	}

	// Utility functions for removing items from std::vector without maintaining order.

	template<typename VectorItemType, typename Alc>
	void RemoveSwap(std::vector<VectorItemType,Alc>& vector, int indexToRemove)
	{
		assert(indexToRemove < vector.size());
		vector[indexToRemove] = std::move(vector.back());
		vector.pop_back();
	}

	template<typename VectorItemType, typename Alc>
	void RemoveSwap(std::vector<VectorItemType,Alc>& vector, typename std::vector<VectorItemType,Alc>::iterator& itrToRemove)
	{
		assert(itrToRemove != vector.cend());
		auto backItr = vector.end() - 1;
		*itrToRemove = std::move(*backItr);
		vector.pop_back();
	}

	template<typename VectorItemType, typename Alc, typename ValueType>
	void RemoveSwapFirstMatchingItem(std::vector<VectorItemType,Alc>& vector, ValueType& value)
	{
		auto itr = Find(vector, value);
		if (itr != vector.end())
		{
			RemoveSwap(vector, itr);
		}
	}

	template<typename VectorItemType, typename Alc, typename ValueType>
	void RemoveSwapAllMatchingItems(std::vector<VectorItemType,Alc>& vector, ValueType& value)
	{
		for (int i = 0; i < vector.size(); ++i)
		{
			if (vector[i] == value)
			{
				RemoveSwap(vector, i);
				--i;
			}
		}
	}

	template<typename VectorItemType, typename Alc>
	void RemoveSwapAllNullItems(std::vector<VectorItemType,Alc>& vector)
	{
		for (int i = 0; i < vector.size(); ++i)
		{
			if (vector[i] == nullptr)
			{
				RemoveSwap(vector, i);
				--i;
			}
		}
	}
	
	// Utility function for inserting with simpler syntax

	template<typename VectorItemType, typename Alc>
	void InsertAt(std::vector<VectorItemType,Alc>& vector, int indexToInsert, VectorItemType item)
	{
		vector.insert(vector.begin() + indexToInsert, item);
	}

	
	// Usable only if an IsValid function has been defined for the item type.
	//template<typename VectorItemType>
	//void RemoveSwapAllInvalidItems(std::vector<VectorItemType,Alc>& vector)
	//{
	//	for (int i = 0; i < vector.size(); ++i)
	//	{
	//		const VectorItemType& item = vector[i];
	//		if (!IsValid(item))
	//		{
	//			RemoveSwap(vector, i);
	//			--i;
	//		}
	//	}
	//}

	template<typename VectorItemType, typename Alc>
	void Fill(std::vector<VectorItemType,Alc>& vector, int num, VectorItemType value)
	{
		vector.resize(num);
		for (int i = 0; i < num; ++i)
		{
			vector[i] = value;
		}
	}

	template<typename VectorItemType, typename Alc>
	void FillAscending(std::vector<VectorItemType,Alc>& vector, int num, VectorItemType startAt)
	{
		VectorItemType counter = startAt;

		vector.clear();
		vector.reserve(num);
		for (int i = 0; i < num; ++i)
		{
			vector.push_back(counter);
			++counter;
		}
	}

	template<typename VectorItemType, typename Alc>
	VectorItemType PopBack(std::vector<VectorItemType,Alc>& vector)
	{
		VectorItemType result = vector.back();
		vector.pop_back();
		return result;
	}

	template<typename VectorItemType, typename Alc>
	void AddUnique(std::vector<VectorItemType,Alc>& vec, const VectorItemType& item)
	{
		if (!Contains(vec, item))
		{
			vec.push_back(item);
		}
	}

	// Adds the second vector to the end of the first vector.  Second vector is unchanged.
	template<typename VectorItemType, typename Alc>
	void Append(std::vector<VectorItemType,Alc>& first, const std::vector<VectorItemType,Alc>& second)
	{
		first.insert(first.end(), second.begin(), second.end());
	}

	// Sorts the list with std::sort.
	// Ascending requires operator< while descending requires operator>.
	template<typename VectorItemType, typename Alc>
	void SortAscending(std::vector<VectorItemType,Alc>& v)
	{
		std::sort(v.begin(), v.end());
	}
	template<typename VectorItemType, typename Alc>
	void SortDescending(std::vector<VectorItemType,Alc>& v)
	{
		std::sort(v.begin(), v.end(), std::greater<VectorItemType>());
	}

	template<typename VectorItemType, typename Alc>
	void StableSortAscending(std::vector<VectorItemType,Alc>& v)
	{
		std::stable_sort(v.begin(), v.end());
	}
	template<typename VectorItemType, typename Alc>
	void StableSortDescending(std::vector<VectorItemType,Alc>& v)
	{
		std::stable_sort(v.begin(), v.end(), std::greater<VectorItemType>());
	}
	
	// Get a list of the indices in another vector.
	template<typename VectorItemType, typename Alc>
	IntTempList GetIndices(const std::vector<VectorItemType,Alc>& v)
	{
		IntTempList t;
		FillAscending(t, Size(v), 0);
		return t;
	}
}
