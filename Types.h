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

class Map;

struct Vec2;
struct Box;

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

namespace Miscast
{
	enum Category : int;
}

namespace Player
{
	struct Data;
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

enum class GameMode : byte;
enum class Gender : byte;
enum class MoveMode : byte;
enum class TargetMode : byte;
enum class Terrain : byte;
enum class Visibility : byte;

namespace Miscast
{
	enum Category : int;
}
