#pragma once

#include "Debug.h"
#include "Types.h"

#include <array>
#include <bitset>

template<typename T,int size>
class SparseArray
{
public:
	//using T = int;
	//static int constexpr size = 10;

	SparseArray()
	{
		clear();
	}

	void fill (T const& value)
	{
		for (int i = 0; i < size; ++i)
		{
			data.fill(value);
		}
	}

	void clear ()
	{
		free_space = size;
		mask.reset();
		fill(T{});
	}

	int is_empty () const
	{
		return free_space == size;
	}

	bool has_space () const
	{
		return free_space > 0;
	};

	int num_used () const
	{
		return size - free_space;
	}

	int num_free () const
	{
		return free_space;
	}

	bool is_used (int index) const
	{
		return mask.test(index);
	}

	bool is_free (int index) const
	{
		return !mask.test(index);
	}

	// Add new item in the first free space.
	// Returns new index.  If no free space, returns c_Invalid.
	int insert (T const& value)
	{
		int const index = find_free_cell();
		if (index != c_Invalid)
		{
			mask.set(index, true);
			set(index, value);
		}
		return index;
	}

	int insert (T&& value)
	{
		int const index = find_free_cell();
		if (index != c_Invalid)
		{
			mask.set(index, true);
			data[index] = std::move(value);
		}
		return index;
	}

	void remove (int index)
	{
		set(index, T{});
		mask.set(index, false);
	}

	T extract (int index)
	{
		assert(index >= 0 && index < size);
		mask.set(index, false);
		return std::move(data[index]);
	}

	T const& at (int index)
	{
		return operator[](index);
	}

	void set(int index, T const& value)
	{
		operator[](index) = value;
	}

	T& operator[] (int index)
	{
		assert(index >= 0 && index < size);
		assert(mask.test(index));
		return data[index];
	}

	// Serialize the sparse array, assuming T is a simple value type.
	//void serialize(ISerializer& s)
	//{
	//	int n = size;
	//	s.srz_int(n);
	//	if (n != size)
	//	{
	//		DebugBreak("SparseArray - Serialize failed - Size mismatch");
	//		return;
	//	}
	//
	//	s.srz_value(mask);
	//	for (int i = 0; i < size; ++i)
	//	{
	//		if (mask.test(i))
	//		{
	//			s.srz_value(at(i));
	//		}
	//		else if (s.is_load())
	//		{
	//			data[i] = T{};
	//		}
	//	}
	//}

protected:
	int find_free_cell ()
	{
		for (int i = 0; i < size; ++i)
		{
			// This would be more efficient if we first checked one byte at a time or something,
			// but I don't think std::bitset supports that.
			if (!mask.test(i))
			{
				return i;
			}
		}

		return c_Invalid;
	}

	int free_space = size;
	std::bitset<size> mask;
	std::array<T,size> data;
};
