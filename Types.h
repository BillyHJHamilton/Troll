#pragma once

#include <vector>

static int constexpr c_invalid = -1;

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
struct Vec3;
struct Box2;

enum class GameMode : byte;
enum class Gender : byte;
enum class MoveMode : byte;
enum class RoomType : byte;
enum class TargetMode : byte;
enum class Visibility : byte;

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
}

namespace Stairs
{
	enum Direction : byte;
}

namespace Status
{
	enum Index : int;
}

namespace Terrain
{
	enum Type : byte;
}
