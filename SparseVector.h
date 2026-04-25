#pragma once

#include "Debug.h"
#include "Serialize.h"
#include "VectorUtil.h"

// A sparse array that dynamically grows if it runs out of space.
// Implemented as a wrapper around std::vector with a std::vector<bool> for an occupancy mask.
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

	bool is_free (int index) const
	{
		return Util::IsValidIndex(mask, index) && !mask.at(index);
	}

	bool is_valid (int index) const
	{
		return Util::IsValidIndex(mask, index) && mask.at(index);
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
		if (mask.at(index))
		{
			mask.at(index) = false;
			data.at(index) = T{};
			++free_space;
		}
	}

	T extract (int index)
	{
		mask.at(index) = false;
		++free_space;
		return std::move(data[index]);
	}

	T const& read (int index) const
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

	//-------------------------------------------------------------------------
	// Iterators

	class ConstItr
	{
	public:
		ConstItr(SparseVector<T> const& vector, int index = 0) : v(vector), i(index)
		{
			seek();
		}

		void advance () { ++i; seek(); }
		bool finished () const { return i >= v.size() || i < 0; }
		bool valid () const { return !finished(); }
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
		operator bool() const { return valid(); }
		// post-increment not provided to avoid accidental copy

	private:
		void seek()
		{
			while (i < v.size() && v.is_free(i))
			{
				++i;
			}
		}

		SparseVector<T> const& v;
		int i;
	};

	class Itr
	{
	public:
		Itr(SparseVector<T>& vector, int index = 0) : v(vector), i(index)
		{
			seek();
		}
		
		void advance () { ++i; seek(); }
		bool finished () const { return i >= v.size() || i < 0; }
		bool valid () const { return !finished(); }
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
		operator bool() const { return valid(); }
		// post-increment not provided to avoid accidental copy

	private:
		void seek()
		{
			while (i < v.size() && v.is_free(i))
			{
				++i;
			}
		}

		SparseVector<T>& v;
		int i;
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
