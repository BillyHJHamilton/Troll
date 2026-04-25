#pragma once

#include "Debug.h"
#include "Serialize.h"
#include "Types.h"
#include "VectorUtil.h"

// A sparse array that dynamically grows if it runs out of space.
// Implemented as a wrapper around std::vector with a std::vector<bool> for an occupancy mask.
template<typename T>
class SparseVector
{
public:
	SparseVector()
	{
		clear();
	}

	void reserve (int size)
	{
		mask.reserve(size);
		data.reserve(size);
	}

	void clear ()
	{
		mask.clear();
		data.clear();
		free_space = 0;
	}

	int is_empty () const
	{
		return free_space == size();
	}

	int num_used () const
	{
		return Util::Size(mask) - free_space;
	}

	// Gives the size, including free spaces.
	int size() const
	{
		return Util::Size(mask);
	}

	bool is_used (int index) const
	{
		return mask.at(index);
	}

	bool is_free (int index) const
	{
		return !mask.at(index);
	}

	// Add new item in the first free space.  Returns new index.
	int insert (T const& value)
	{
		int const index = find_free_cell();
		mask.at(index) = true;
		data.at(index) = value;
		--free_space;

		return index;
	}

	int insert (T&& value)
	{
		int const index = find_free_cell();
		mask.at(index) = true;
		data.at(index) = std::move(value);
		--free_space;

		return index;
	}

	void remove (int index)
	{
		mask.at(index) = false;
		data.at(index) = T{};
		++free_space;
	}

	T extract (int index)
	{
		mask.at(index) = false;
		++free_space;
		return std::move(data[index]);
	}

	T const& read (int index)
	{
		assert(mask.at(index));
		return data.at(index);
	}

	T& edit (int index)
	{
		assert(mask.at(index));
		return data.at(index);
	}

	void set(int index, T const& value)
	{
		if (!mask.at(index))
		{
			--free_space;
			mask.at(index) = true;
		}
		operator[](index) = value;
	}

	T& operator[] (int index)
	{
		assert(mask.at(index));
		return data.at(index);
	}

	// Serialize the sparse array, assuming T is a simple value type.
	void serialize(ISerializer& s)
	{
		serialize_size_and_mask(s);

		for (int i = 0; i < size(); ++i)
		{
			if (mask.at(i))
			{
				s.srz_value(data.at(i));
			}
		}
	}

	// For a complex type, call this and then handle the data yourself.
	void serialize_size_and_mask(ISerializer& s)
	{
		s.srz_int(free_space);

		int n = size();
		s.srz_int(n);

		if (s.is_load())
		{
			mask.resize(n, false);
			data.resize(n, T{});
		}

		for (int i = 0; i < size(); ++i)
		{
			// Deal with the annoyingness of std::vector<bool>
			if (s.is_load())
			{
				bool b;
				s.srz_bool(b);
				mask.at(i) = b;
			}
			else
			{
				bool b = mask.at(i);
				s.srz_bool(b);
			}
		}
	}

protected:
	int find_free_cell ()
	{
		if (free_space > 0)
		{
			for (int i = 0; i < Util::Size(data); ++i)
			{
				if (!mask.at(i))
				{
					return i;
				}
			}
			DebugBreak("We expected to find a free space here.");
		}
		
		// Out of space, so add to the end.
		mask.push_back(false);
		data.push_back(T{});
		free_space += 1;
		assert(mask.size() == data.size());
		return Util::LastIndex(data);
	}

	int free_space = 0;
	std::vector<bool> mask;
	std::vector<T> data;
};
