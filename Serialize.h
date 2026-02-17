#pragma once

#include "Grid.h"
#include "NameHash.h"
#include "Types.h"

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
	virtual void srz_int_vec(std::vector<int>& v) = 0;
	virtual void srz_int_grid(Grid<int>& g) = 0;

	virtual void srz_int_int_hashmap(std::unordered_map<int,int>& m) = 0;
	virtual void srz_vec2_int_hashmap(std::unordered_map<Vec2,int>& m) = 0;
	virtual void srz_vec2_stairs_hashmap(std::unordered_map<Vec2,Stairs::Direction>& m) = 0;
};

template<typename ValueType>
void srz_vec_size(ISerializer& s, std::vector<ValueType> v)
{
	int vec_size;
	if (s.is_load())
	{
		s.srz_int(vec_size);
		if ((int)v.size() != vec_size)
		{
			v.resize(vec_size);
		}
	}
	else
	{
		vec_size = (int)v.size();
		s.srz_int(vec_size);
	}
}

template<typename ValueType>
void srz_grid_size(ISerializer& s, Grid<ValueType> g)
{
	if (s.is_load())
	{
		int w, h;
		s.srz_int(w);
		s.srz_int(h);
		if (g.get_width() != w || g.get_height() != h)
		{
			g = Grid<ValueType>(w,h,{});
		}
	}
	else
	{
		int w = g.get_width();
		int h = g.get_height();
		s.srz_int(w);
		s.srz_int(h);
	}
}
