#pragma once

#include "Debug.h"
#include "Serialize.h"
#include "VectorUtil.h"

// A sparse array that dynamically grows if it runs out of space.
// It tracks which slots are used, and has a counter at each slot so it is
// possible to check if the same slot is still occupied by the same element.
// Requires T to have a default constructor; default is stored in free spaces.
template<typename T>
class SparseVector
{
public:
	class Itr;
	class ConstItr;

	//-------------------------------------------------------------------------
	// Main operations

	SparseVector()
	{
		clear();
	}

	void reserve (int size)
	{
		counter.reserve(size);
		data.reserve(size);
	}

	void clear ()
	{
		counter.clear();
		data.clear();
		free_space = 0;
	}

	int is_empty () const
	{
		return free_space == size();
	}

	int num_used () const
	{
		return Util::Size(counter) - free_space;
	}

	// Gives the size, including free spaces.
	int size() const
	{
		return Util::Size(counter);
	}

	bool is_free (int index) const
	{
		return (counter.at(index) % 2) == 0;
	}

	bool is_valid (int index) const
	{
		return is_in_range(index) && !is_free(index);
	}

	// Checks whether the index is within the range of the container,
	// but not whether that slot is currently occupied.
	bool is_in_range (int index) const
	{
		return Util::IsValidIndex(counter, index);
	}

	// Add new item in the first free space.  Returns new index.
	int insert (T const& value)
	{
		int const index = find_free_cell();
		occupy(index);
		data.at(index) = value;

		return index;
	}

	int insert (T&& value)
	{
		int const index = find_free_cell();
		occupy(index);
		data.at(index) = std::move(value);

		return index;
	}

	void remove (int index)
	{
		if (!is_free(index))
		{
			++counter.at(index);
			data.at(index) = T{};
			assert(is_free(index));
		}
	}

	T const& read (int index) const
	{
		assert(is_valid(index));
		return data.at(index);
	}

	T& edit (int index)
	{
		assert(is_valid(index));
		return data.at(index);
	}

	void set(int index, T const& value)
	{
		occupy(index);
		data.at(index) = value;
	}

	T& operator[] (int index)
	{
		assert(is_valid(index));
		return data.at(index);
	}

	uint16_t get_counter(int index) const
	{
		if (is_in_range(index))
		{
			return counter.at(index);
		}
		return 0u;
	}
	
	//-------------------------------------------------------------------------
	// Advanced operations

	template<typename KeyType>
	int find_index_by_key(KeyType T::* key_variable, KeyType key_to_find) const
	{
		for (ConstItr itr = begin(); itr; ++itr)
		{
			if ((*itr).*key_variable == key_to_find)
			{
				return itr.index();
			}
		}
		return c_Invalid; // key not found
	}

	//-------------------------------------------------------------------------
	// Serialization

	// Serialize the sparse array, assuming T is a simple value type.
	void serialize(ISerializer& s)
	{
		serialize_size_and_mask(s);
	
		for (int i = 0; i < size(); ++i)
		{
			if (is_valid(i))
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
			counter.resize(n, false);
			data.resize(n, T{});
		}

		s.srz_array_data(counter.data(), n);
	}

	//---------------------------------------------------------------------------------------------
	// Iterators

	class ConstItr
	{
	public:
		ConstItr(SparseVector<T> const& vector, int index = 0) : v(vector), i(index), c(0)
		{
			seek();
		}

		void advance () { ++i; seek(); }
		bool valid () const { return v.is_valid(i) && v.get_counter(i) == c; }
		SparseVector const* address() const { return &v; }
		int index() const { return i; }

		// iterator functions
		T const & operator*() const { return v.read(i); }
		T const * operator->() const { return &v.read(i); }
		ConstItr& operator++() { advance(); return *this; }
		bool operator!= (SparseVector::Itr rhs) const { return address() != rhs.address() ||
			index() != rhs.index(); }
		bool operator!= (ConstItr rhs) const { return address() != rhs.address() ||
			index() != rhs.index(); }
		explicit operator bool() const { return valid(); }
		// post-increment not provided to avoid accidental copy

	private:
		void seek()
		{
			while (i < v.size() && v.is_free(i))
			{
				++i;
			}

			c = v.get_counter(i);
		}

		SparseVector<T> const& v;
		int i;
		uint16_t c;
	};

	class Itr
	{
	public:
		Itr(SparseVector<T>& vector, int index = 0) : v(vector), i(index), c(0)
		{
			seek();
		}

		void advance () { ++i; seek(); }
		bool valid () const { return v.is_valid(i) && v.get_counter(i) == c; }
		SparseVector const* address() const { return &v; }
		int index() const { return i; }

		// Removes item at the iterator.  This temporarily invalidates the iterator, but
		// it keeps its index and will become valid again the next time it is advanced.
		void remove_current () { v.remove(i); }

		// iterator functions
		T & operator*() const { return v.edit(i); }
		T * operator->() const { return &v.edit(i); }
		Itr& operator++() { advance(); return *this; }
		bool operator!= (Itr rhs) const { return address() != rhs.address() ||
			index() != rhs.index(); }
		bool operator!= (ConstItr rhs) const { return address() != rhs.address() ||
			index() != rhs.index(); }
		explicit operator bool() const { return valid(); }
		// post-increment not provided to avoid accidental copy

	private:
		void seek()
		{
			while (i < v.size() && v.is_free(i))
			{
				++i;
			}

			c = v.get_counter(i);
		}

		SparseVector<T>& v;
		int i;
		uint16_t c;
	};

	Itr begin() { return Itr(*this); }
	Itr end() { return Itr(*this, size()); }
	ConstItr begin() const { return ConstItr(*this); }
	ConstItr end() const { return ConstItr(*this, size()); }
	ConstItr cbegin() const { return ConstItr(*this); }
	ConstItr cend() const { return ConstItr(*this, size()); }

	Itr get_itr(int index) { return Itr(*this, index); }
	ConstItr get_const_itr(int index) { return ConstItr(*this, index); }

protected:
	int find_free_cell ()
	{
		if (free_space > 0)
		{
			for (int i = 0; i < Util::Size(data); ++i)
			{
				if (is_free(i))
				{
					return i;
				}
			}
			DebugBreak("We expected to find a free space here.");
		}
		
		// Out of space, so add to the end.
		counter.push_back(0u);
		data.push_back(T{});
		++free_space;
		assert(counter.size() == data.size());
		return Util::LastIndex(data);
	}

	void occupy (int index)
	{
		if (is_free(index))
		{
			++counter.at(index);
			--free_space;
		}
	}

	int free_space = 0;

	// Even value means space is free, odd means it is occupied.
	// Incremented each time an element is added/removed in the slot.
	// Can be used to check if iterator still refers to same element.
	std::vector<uint16_t> counter;

	std::vector<T> data;
};

