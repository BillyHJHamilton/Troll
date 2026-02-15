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

class IMenu;

class Map;
class MapGenerator;
class World;

struct Vec2;
struct Vec3;
struct Box2;

enum CompassDirection : int;

enum class GameMode : byte;
enum class Gender : byte;
enum class MoveMode : byte;
enum class RoomType : byte;
enum class TargetMode : byte;
enum class Visibility : byte;

namespace Cloud
{
	enum Type : int;
	struct Instance;
}

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

namespace House
{
	enum Type : int;
}

namespace Item
{
	enum Type : int;
	struct Instance;
	class Handle;
}

namespace LineCache
{
	class Itr;
	class Itr3D;
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
	struct EffectParams;
	using EffectFunc = void(*)(EffectParams params);
}

namespace Stairs
{
	enum Direction : byte;
	using Pair = std::pair<Vec2,Stairs::Direction>;
}

namespace Status
{
	enum Index : int;
}

namespace Terrain
{
	enum Type : byte;
}
