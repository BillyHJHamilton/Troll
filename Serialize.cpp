#include "Serialize.h"

#include "Game.h"
#include "Creature.h"
#include "Geometry.h"
#include "MapUtil.h"
#include "Item.h"
#include "VectorUtil.h"

#include <fstream>

// Oh boy

class Saver : public ISerializer
{
public:
	Saver(const std::string& filename) :
		fout(filename, std::ios::binary)
	{}

	template<typename T>
	void write(T t)
	{
		fout.write( (char*) &t, sizeof(t) );
	}

	template<typename KeyType, typename ValueType>
	void write_map(std::unordered_map<KeyType,ValueType>& m, char const* debug_name)
	{
		write(Util::Size(m));
		if (c_ShowSerializeDebug)
		{
			std::cout << std::format("Save {}, size={}\n", debug_name, Util::Size(m));
		}
		for (auto& pair : m)
		{
			write(pair.first);
			write(pair.second);
		}
	}

	virtual bool is_load() const override { return false; }

	virtual void srz_raw(char* c, int size) override
	{
		fout.write(c, size);
	}

	virtual void srz_bool(bool& b) override { write(b); }
	virtual void srz_int(int& x) override { write(x); }
	virtual void srz_float(float& f) override { write(f); }
	virtual void srz_byte(byte& b) override { write(b); }
	virtual void srz_char(char& c) override { write(c); }

	virtual void srz_vec2(Vec2& v) override { write(v); }
	virtual void srz_vec3(Vec3& v) override { write(v); }
	virtual void srz_box2(Box2& b) override { write(b); }

	virtual void srz_name_hash(NameHash& h) override { write(h); }
	virtual void srz_creature_handle(Creature::Handle& h) override { write(h); }
	virtual void srz_item_handle(Item::Handle& h) override { write(h); }

	virtual void srz_string(std::string& str) override
	{
		int const str_size = (int)str.size();
		write(str_size);
		fout.write(str.data(), str_size);
	}

	virtual void srz_int_int_hashmap(std::unordered_map<int,int>& m,
		char const* debug_name) override
	{
		write_map(m, debug_name);
	}

	virtual void srz_vec2_int_hashmap(std::unordered_map<Vec2,int>& m,
		char const* debug_name) override
	{
		write_map(m, debug_name);
	}

	virtual void srz_vec2_stairs_hashmap(std::unordered_map<Vec2,Stairs::Direction>& m,
		char const* debug_name) override
	{
		write_map(m, debug_name);
	}

protected:
	std::ofstream fout;
};

class Loader : public ISerializer
{
public:
	Loader(const std::string& filename) :
		fin(filename, std::ios::binary)
	{}
	
	template<typename T>
	void read(T& t)
	{
		fin.read( (char*) &t, sizeof(t) );
	}

	template<typename KeyType, typename ValueType>
	void read_map(std::unordered_map<KeyType,ValueType>& m, char const* debug_name)
	{
		int map_size;
		read(map_size);
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
			read(a);
			read(b);
			m.emplace(a,b);
		}
	}

	virtual bool is_load() const override { return true; }

	virtual void srz_raw(char* c, int size) override
	{
		fin.read(c, size);
	}

	virtual void srz_bool(bool& b) override { read(b); }
	virtual void srz_int(int& x) override { read(x); }
	virtual void srz_float(float& f) override { read(f); }
	virtual void srz_byte(byte& b) override { read(b); }
	virtual void srz_char(char& c) override { read(c); }

	virtual void srz_vec2(Vec2& v) override { read(v); }
	virtual void srz_vec3(Vec3& v) override { read(v); }
	virtual void srz_box2(Box2& v) override { read(v); }

	virtual void srz_name_hash(NameHash& h) override { read(h); }
	virtual void srz_creature_handle(Creature::Handle& h) override { read(h); }
	virtual void srz_item_handle(Item::Handle& h) override { read(h); }

	virtual void srz_string(std::string& str) override
	{
		int str_size;
		read(str_size);
		str.resize(str_size);
		fin.read(str.data(), str_size);
	}

	virtual void srz_int_int_hashmap(std::unordered_map<int,int>& m,
		char const* debug_name) override
	{
		read_map(m, debug_name);
	}
	
	virtual void srz_vec2_int_hashmap(std::unordered_map<Vec2,int>& m, char const*
		debug_name) override
	{
		read_map(m, debug_name);
	}

	virtual void srz_vec2_stairs_hashmap(std::unordered_map<Vec2,Stairs::Direction>& m,
		char const* debug_name) override
	{
		read_map(m, debug_name);
	}

protected:
	std::ifstream fin;
};

//-------------------------------------------------------------------------------------------------
// Global Interface

void SaveGame(const std::string& filename)
{
	Saver s(filename);
	Game::serialize_all(s);
}

void LoadGame(const std::string& filename)
{
	Loader s(filename);
	Game::serialize_all(s);
}
