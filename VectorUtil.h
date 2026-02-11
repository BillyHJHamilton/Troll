#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace Util
{
	// Get the size and convert to int (to suppress size_t warnings).
	template<class T>
	inline int Size(std::vector<T> const& vector)
	{
		return static_cast<int>(vector.size());
	}

	// Get index of the last item added to the back of the vector.
	template<class T>
	inline int LastIndex(std::vector<T> const& vector)
	{
		return Size(vector) - 1;
	}

	template<typename VectorItemType>
	bool IsValidIndex(std::vector<VectorItemType> const& vector, int index)
	{
		return index >= 0 && index < vector.size();
	}

	// Basic functions for searching.

	template<typename VectorItemType, typename ValueType>
	auto Find(std::vector<VectorItemType> const& vector, ValueType value)
	{
		return std::find(vector.begin(), vector.end(), value);
	}

	template<typename VectorItemType, typename ValueType>
	bool Contains(std::vector<VectorItemType> const& vector, ValueType& value)
	{
		auto itr = std::find(vector.begin(), vector.end(), value);
		return itr != vector.end();
	}

	// Functions for searching an array of unique_ptrs when you have a normal pointer.

	template<typename PointerType>
	auto Find(std::vector<std::unique_ptr<PointerType>> const& vector, PointerType* pointer)
	{
		return std::find_if(vector.begin(), vector.end(),
			[pointer](std::unique_ptr<PointerType>& entry)
			{
				return entry.get() == pointer;
			}
		);
	}

	template<typename PointerType>
	bool Contains(std::vector<std::unique_ptr<PointerType>>& vector, PointerType* pointer)
	{
		auto itr = Find(vector, pointer);
		return itr != vector.end();
	}

	// Utility functions for removing with simpler syntax

	template<typename VectorItemType>
	void RemoveAt(std::vector<VectorItemType>& vector, int indexToRemove)
	{
		vector.erase(vector.begin() + indexToRemove);
	}

	template<typename VectorItemType, typename ValueType>
	void RemoveFirstMatchingItem(std::vector<VectorItemType>& vector, ValueType& value)
	{
		auto itr = Find(vector, value);
		if (itr != vector.end())
		{
			vector.erase(itr);
		}
	}
	
	template<typename VectorItemType, typename ValueType>
	void RemoveAllMatchingItems(std::vector<VectorItemType>& vector, ValueType& value)
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

	template<typename VectorItemType>
	void RemoveSwap(std::vector<VectorItemType>& vector, int indexToRemove)
	{
		assert(indexToRemove < vector.size());
		vector[indexToRemove] = std::move(vector.back());
		vector.pop_back();
	}

	template<typename VectorItemType>
	void RemoveSwap(std::vector<VectorItemType>& vector, typename std::vector<VectorItemType>::iterator& itrToRemove)
	{
		assert(itrToRemove != vector.cend());
		auto backItr = vector.end() - 1;
		*itrToRemove = std::move(*backItr);
		vector.pop_back();
	}

	template<typename VectorItemType, typename ValueType>
	void RemoveSwapFirstMatchingItem(std::vector<VectorItemType>& vector, ValueType& value)
	{
		auto itr = Find(vector, value);
		if (itr != vector.end())
		{
			RemoveSwap(vector, itr);
		}
	}

	template<typename VectorItemType, typename ValueType>
	void RemoveSwapAllMatchingItems(std::vector<VectorItemType>& vector, ValueType& value)
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

	template<typename VectorItemType>
	void RemoveSwapAllNullItems(std::vector<VectorItemType>& vector)
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
	
	// Usable only if an IsValid function has been defined for the item type.
	//template<typename VectorItemType>
	//void RemoveSwapAllInvalidItems(std::vector<VectorItemType>& vector)
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

	template<typename VectorItemType>
	VectorItemType PopBack(std::vector<VectorItemType>& vector)
	{
		VectorItemType result = vector.back();
		vector.pop_back();
		return result;
	}

	template<typename VectorItemType>
	void AddUnique(std::vector<VectorItemType>& vec, const VectorItemType& item)
	{
		if (!Contains(vec, item))
		{
			vec.push_back(item);
		}
	}

	// Adds the second vector to the end of the first vector.  Second vector is unchanged.
	template<typename VectorItemType>
	void Append(std::vector<VectorItemType>& first, const std::vector<VectorItemType>& second)
	{
		first.insert(first.end(), second.begin(), second.end());
	}

	// Sorts the list with std::sort.
	// Ascending requires operator< while descending requires operator>.
	template<typename VectorItemType>
	void SortAscending(std::vector<VectorItemType>& v)
	{
		std::sort(v.begin(), v.end());
	}
	template<typename VectorItemType>
	void SortDescending(std::vector<VectorItemType>& v)
	{
		std::sort(v.begin(), v.end(), std::greater<VectorItemType>());
	}
}
