#include "Serialize.h"

#include "Game.h"

#include <fstream>

// Oh boy

class Saver : public ISerializer
{
public:
	Saver(const std::string& filename) :
		fout(filename, std::ios::binary)
	{}

	virtual bool is_load() const override { return false; }

	virtual void srz_raw(char* c, int size) override
	{
		fout.write(c, size);
	}

	virtual void srz_string(std::string& str) override
	{
		int str_size = (int)str.size();
		srz_int(str_size);
		fout.write(str.data(), str_size);
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

	virtual bool is_load() const override { return true; }

	virtual void srz_raw(char* c, int size) override
	{
		fin.read(c, size);
	}

	virtual void srz_string(std::string& str) override
	{
		int str_size;
		srz_int(str_size);
		str.resize(str_size);
		fin.read(str.data(), str_size);
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
