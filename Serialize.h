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
	virtual bool is_load() const = 0;

	virtual void srz_raw(char* c, int size) = 0;

	virtual void srz_bool(bool& b) = 0;
	virtual void srz_int(int& x) = 0;
	virtual void srz_float(float& f) = 0;
	virtual void srz_byte(byte& b) = 0;
	virtual void srz_char(char& c) = 0;

	virtual void srz_vec2(Vec2& v) = 0;
	virtual void srz_vec3(Vec3& v) = 0;
	virtual void srz_box2(Box2& b) = 0;

	virtual void srz_name_hash(NameHash& h) = 0;
	virtual void srz_creature_handle(Creature::Handle& h) = 0;
	virtual void srz_item_handle(Item::Handle& h) = 0;

	virtual void srz_string(std::string& str) = 0;
};

// Generic function to serialize raw data.  Good for stable structs, enums and the like.
// Obviously NOT safe for things with pointers or dynamic data.
template<typename ValueType>
void srz_value(ISerializer& s, ValueType& x)
{
	s.srz_raw((char*)&x, sizeof(x));
}

// For serializing a vector of raw data, call srz_vector.

// If your vector is complex, you call srz_vector_size,
// then iterate over the vector and serialize each element with custom logic.
// Make sure not to serialize a COPY of the elements, or nothing will load!

template<typename ValueType>
void srz_vector_size(ISerializer& s, std::vector<ValueType>& v, char const* debug_name)
{
	int vec_size;
	if (s.is_load())
	{
		s.srz_int(vec_size);
		if (c_ShowSerializeDebug)
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
		s.srz_int(vec_size);
		if (c_ShowSerializeDebug)
		{
			std::cout << "Save " << debug_name << ", size " << vec_size << "\n";
		}
	}
}

template<typename ValueType>
void srz_array_data(ISerializer& s, ValueType* raw_data, int num)
{
	s.srz_raw((char*)raw_data, num * sizeof(ValueType));
}

template<typename ValueType>
void srz_vector(ISerializer& s, std::vector<ValueType>& v, char const* debug_name)
{
	srz_vector_size(s, v, debug_name);
	srz_array_data(s, v.data(), (int)v.size());
}

template<typename ValueType>
void srz_grid_size(ISerializer& s, Grid<ValueType>& g, char const* debug_name)
{
	if (s.is_load())
	{
		int w, h;
		s.srz_int(w);
		s.srz_int(h);
		bool resized = false;
		if (g.get_width() != w || g.get_height() != h)
		{
			g = Grid<ValueType>(w,h,{});
			resized = true;
		}
		int const length = w * h;
		if (c_ShowSerializeDebug)
		{
			std::cout << std::format("Load {} - w={}, h={}, resized={}, length={}\n",
				debug_name, w, h, resized, length);
		}
	}
	else
	{
		int w = g.get_width();
		int h = g.get_height();
		s.srz_int(w);
		s.srz_int(h);
		int const length = w * h;
		if (c_ShowSerializeDebug)
		{
			std::cout << std::format("Save {} - w={}, h={}; length={}\n",
				debug_name, w, h, length);
		}
	}
}

template<typename ValueType>
void srz_grid_data(ISerializer& s, Grid<ValueType>& g)
{
	s.srz_raw((char*)g.edit_data().data(), g.num() * sizeof(ValueType));
}

template<typename ValueType>
void srz_grid(ISerializer& s, Grid<ValueType>& g, char const* debug_name)
{
	srz_grid_size(s, g, debug_name);
	srz_grid_data(s, g);
}

// Serialize a map with value types.
template<typename KeyType, typename ValueType>
void srz_hashmap(ISerializer& s, std::unordered_map<KeyType,ValueType> m,
	char const* debug_name)
{
	if (s.is_load())
	{
		int map_size;
		s.srz_int(map_size);
		m.clear();
		m.reserve(map_size);
		if (c_ShowSerializeDebug)
		{
			std::cout << std::format("Load {}, size={}\n", debug_name, map_size);
		}
		for (int i = 0; i < map_size; ++i)
		{
			KeyType a;
			ValueType b;
			srz_value(s, a);
			srz_value(s, b);
			m.emplace(a,b);
		}
	}
	else
	{
		int map_size = (int)m.size();
		s.srz_int(map_size);
		if (c_ShowSerializeDebug)
		{
			std::cout << std::format("Save {}, size={}\n", debug_name, map_size);
		}
		for (auto& pair : m)
		{
			srz_value(s, pair.first);
			srz_value(s, pair.second);
		}
	}
}
