#pragma once

#include <vector>

using byte = unsigned char;

template<class T>
using Grid = std::vector<std::vector<T>>;

template<class T>
Grid<T> make_grid (int x, int y, T value)
{
	return std::vector<std::vector<T>>(x, std::vector<T>(y, value));
}

struct Vec2;
struct Box;
struct Map;
struct Player;

namespace Creature
{
	enum Type : int;
	struct Stats;
	struct DerivedStats;
	class Handle;
}

namespace Draw
{
	struct View;
}

namespace Menu
{
	enum Id : int;
}

namespace Spell
{
	enum Index : int;
	using EffectFunc = void(*)(Creature::Handle caster, Creature::Handle target);
};

namespace Status
{
	enum Index : int;
};

enum class Gender;
enum class TargetMode;
enum class Terrain : byte;
enum class Visibility : byte;

namespace Miscast
{
	enum Category : int;
}
