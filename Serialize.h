#pragma once

#include "Debug.h"
#include "Grid.h"
#include "NameHash.h"
#include "Types.h"

#include <format>
#include <string>
#include <unordered_map>
#include <vector>

void SaveGame(const std::string& filename);
void LoadGame(const std::string& filename);

// Using virtual is the lesser evil here.
// I want to keep this header light because it'll be needed everywhere.
class ISerializer
{
public:
	virtual ~ISerializer() {}

	// This is a leaky abstraction, but really useful for writing template functions
	// to handle saving/loading for templated types like maps/vectors.
	virtual bool is_load() const = 0;

	virtual void srz_raw(char* c, int size) = 0;

	// Generic function to serialize raw data.  Good for stable structs, enums and the like.
	// Obviously NOT safe for things with pointers or dynamic data.
	template<typename ValueType>
	void srz_value(ValueType& x)
	{
		srz_raw((char*)&x, sizeof(x));
	}

	// Wrappers for basic value types.
	void srz_bool(bool& b) { srz_value(b); }
	void srz_int(int& x) { srz_value(x); }
	void srz_float(float& f) { srz_value(f); }
	void srz_byte(byte& b) { srz_value(b); }
	void srz_char(char& c) { srz_value(c); }

	void srz_vec2(Vec2& v) { srz_value(v); }
	void srz_vec3(Vec3& v) { srz_value(v); }
	void srz_box2(Box2& b) { srz_value(b); }

	void srz_name_hash(NameHash& h) { srz_value(h); }
	void srz_creature_handle(Creature::Handle& h) { srz_value(h); }
	void srz_item_handle(Item::Handle& h) { srz_value(h); }

	// Types that require special handling.

	virtual void srz_string(std::string& str) = 0;

	// Serializes a vector (of value types) by first serializing the size, then the data.
	// It simply calls srz_vector_size followed by srz_array_data.
	template<typename ValueType>
	void srz_vector(std::vector<ValueType>& v, char const* debug_name);

	// Serializes the size of a vector, but not the contents.
	// If you have a vector of a complex type (shared_ptr for instance),
	// you can call this first, then loop through and handle the values.
	// HINT: Make sure to include & if using a for-each loop!  Otherwise it won't work.
	template<typename ValueType>
	void srz_vector_size(std::vector<ValueType>& v, char const* debug_name);

	// Serializes an array with a known type and size.
	template<typename ValueType>
	void srz_array_data(ValueType* raw_data, int num);

	// Serializes a Grid (of value type) by calling srz_grid_size, then srz_array_data.
	template<typename ValueType>
	void srz_grid(Grid<ValueType>& g, char const* debug_name);

	template<typename ValueType>
	void srz_grid_size(Grid<ValueType>& g, char const* debug_name);

	// Serialize a hash map (if both KeyType and ValueType are simple value types).
	template<typename KeyType, typename ValueType>
	void srz_hashmap(std::unordered_map<KeyType, ValueType>& m, char const* debug_name);
};

//-------------------------------------------------------------------------------------------------
// Template function implementations

template<typename ValueType>
void ISerializer::srz_vector(std::vector<ValueType>& v, char const* debug_name)
{
	srz_vector_size(v, debug_name);
	srz_array_data(v.data(), (int)v.size());
}

template<typename ValueType>
void ISerializer::srz_vector_size(std::vector<ValueType>& v, char const* debug_name)
{
	int vec_size;
	if (is_load())
	{
		srz_int(vec_size);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << "Load " << debug_name << ", size " << vec_size << "\n";
		}
		if ((int)v.size() != vec_size)
		{
			v.resize(vec_size);
		}
	}
	else
	{
		vec_size = (int)v.size();
		srz_int(vec_size);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << "Save " << debug_name << ", size " << vec_size << "\n";
		}
	}
}

template<typename ValueType>
void ISerializer::srz_array_data(ValueType* raw_data, int num)
{
	srz_raw((char*)raw_data, num * sizeof(ValueType));
}

template<typename ValueType>
void ISerializer::srz_grid_size(Grid<ValueType>& g, char const* debug_name)
{
	if (is_load())
	{
		int w, h;
		srz_int(w);
		srz_int(h);
		bool resized = false;
		if (g.get_width() != w || g.get_height() != h)
		{
			g = Grid<ValueType>(w,h,{});
			resized = true;
		}
		int const length = w * h;
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Load {} - w={}, h={}, resized={}, length={}\n",
				debug_name, w, h, resized, length);
		}
	}
	else
	{
		int w = g.get_width();
		int h = g.get_height();
		srz_int(w);
		srz_int(h);
		int const length = w * h;
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Save {} - w={}, h={}; length={}\n",
				debug_name, w, h, length);
		}
	}
}

template<typename ValueType>
void ISerializer::srz_grid(Grid<ValueType>& g, char const* debug_name)
{
	srz_grid_size(g, debug_name);
	srz_array_data(g.edit_data().data(), g.num());
}

// Serialize a map with value types.
template<typename KeyType, typename ValueType>
void ISerializer::srz_hashmap(std::unordered_map<KeyType,ValueType>& m, char const* debug_name)
{
	if (is_load())
	{
		int map_size;
		srz_int(map_size);
		m.clear();
		m.reserve(map_size);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Load {}, size={}\n", debug_name, map_size);
		}
		for (int i = 0; i < map_size; ++i)
		{
			KeyType a;
			ValueType b;
			srz_value(a);
			srz_value(b);
			m.emplace(a,b);
		}
	}
	else
	{
		int map_size = (int)m.size();
		srz_int(map_size);
		if (Debug::enabled(Debug::Serialize))
		{
			std::cout << std::format("Save {}, size={}\n", debug_name, map_size);
		}
		for (auto& pair : m)
		{
			srz_value(pair.first);
			srz_value(pair.second);
		}
	}
}
