#pragma once

#include <type_traits>

struct Vec3;

//------------------------------------------------------------------------------
// Axis names

using Axis = int;
Axis constexpr c_AxisX = 0;
Axis constexpr c_AxisY = 1;
Axis constexpr c_AxisZ = 2;
inline Axis get_other_axis(Axis a) { return 1 - a; } // only really valid for X/Y

//------------------------------------------------------------------------------
// Integer Vec2 class

struct Vec2
{
	union
	{
		struct
		{
			int x;
			int y;
		};

		int data[2];
	};

	int &operator[] (unsigned int n) { return data[n]; }
	const int &operator[] (unsigned int n) const { return data[n]; }

	Vec3 xy0() const; // convert to Vec3 with z of 0
	Vec3 xyz(int z) const; // convert to Vec3 with provided z
};

enum CompassDirection : int
{
	c_CompassInvalid = -1, // this enum is int type so we can do this
	c_CompassEast = 0,
	c_CompassNortheast,
	c_CompassNorth,
	c_CompassNorthwest,
	c_CompassWest,
	c_CompassSouthwest,
	c_CompassSouth,
	c_CompassSoutheast,
	c_CompassNoMove,
	c_CompassCount
};

inline CompassDirection get_clockwise(CompassDirection dir)
{
	return (dir >= c_CompassEast && dir <= c_CompassSoutheast) ?
		(CompassDirection)(((int)dir + 7) % 8) :
		c_CompassInvalid;
}

inline CompassDirection get_counterclockwise(CompassDirection dir)
{
	return (dir >= c_CompassEast && dir <= c_CompassSoutheast) ?
		(CompassDirection)(((int)dir + 1) % 8) :
		c_CompassInvalid;
}

inline CompassDirection get_clockwise_90(CompassDirection dir)
{
	return (dir >= c_CompassEast && dir <= c_CompassSoutheast) ?
		(CompassDirection)(((int)dir + 6) % 8) :
		c_CompassInvalid;
}

inline CompassDirection get_counterclockwise_90(CompassDirection dir)
{
	return (dir >= c_CompassEast && dir <= c_CompassSoutheast) ?
		(CompassDirection)(((int)dir + 2) % 8) :
		c_CompassInvalid;
}

CompassDirection to_compass(Vec2 vec);

// Directions from east, counterclockwise, plus {0,0} at the end.
// Inspired by similar approach in Linley's Dungeon Crawl.
Vec2 constexpr c_Compass [c_CompassCount] =
{
	{ 1, 0},
	{ 1,-1},
	{ 0,-1},
	{-1,-1},
	{-1, 0},
	{-1, 1},
	{ 0, 1},
	{ 1, 1},
	{ 0, 0}
};

inline bool operator== (Vec2 lhs, Vec2 rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!= (Vec2 lhs, Vec2 rhs)
{
	return !(operator==(lhs, rhs));
}

inline Vec2 operator+ (Vec2 lhs, Vec2 rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Vec2 operator- (Vec2 lhs, Vec2 rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline Vec2 operator* (int lhs, Vec2 rhs)
{
	return {lhs * rhs.x, lhs * rhs.y};
}

inline Vec2 operator* (Vec2 lhs, int rhs)
{
	return {lhs.x * rhs, lhs.y * rhs};
}

inline Vec2 operator/ (Vec2 lhs, int rhs)
{
	return {lhs.x / rhs, lhs.y / rhs};
}

inline Vec2 const & operator+= (Vec2 & lhs, Vec2 rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline Vec2 const & operator-= (Vec2 & lhs, Vec2 rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

// no operator*= for int *= Vec2

inline Vec2 const & operator*= (Vec2 & lhs, int rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	return lhs;
}

inline Vec2 const & operator/= (Vec2 & lhs, int rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

// Component-wise vector min and max
Vec2 componentwise_min(Vec2 a, Vec2 b);
Vec2 componentwise_max(Vec2 a, Vec2 b);

// Pythagoras
int square_dist(Vec2 p0, Vec2 p1);

// Checks against squared distance.
// Doesn't include the +0.5 from rounded_range.
bool strict_range(Vec2 p0, Vec2 p1, int max_range);

// Returns true if euclidean distance <= range (by checking square_dist).
bool float_range(Vec2 p0, Vec2 p1, float max_range);

// Checks "float_range" with a float range of max_range + 0.5f.
// This is the standard method for attack and vision ranges in Troll.
inline bool rounded_range(Vec2 p0, Vec2 p1, int max_range)
{
	return float_range(p0, p1, (float)max_range + 0.5f);
}

// Gets the euclidean distance (the square root of square_dist).
// Don't use this if you could use one of the cheaper range functions, of course.
float euclid(Vec2 p0, Vec2 p1);

// Sum of distance in each dimension (i.e., distance with no diagonals allowed).
int manhattan(Vec2 p0, Vec2 p1);

// Distance in shortest dimension (i.e., distance if diagonals cost only 1).
int chessboard(Vec2 p0, Vec2 p1);

inline bool chessboard_adjacent(Vec2 p0, Vec2 p1)
{
	return chessboard(p0,p1) == 1;
}

// Returns the longer axis, or c_AxisX if both are equal.
Axis get_long_axis(Vec2 v);

// Scales both components to -1, 0, or 1, preserving direction as much as possible.
// There are only 9 possible results, all with chessboard dimension <= 1.
// (0, 0) stays as (0, 0).
Vec2 truncate_to_unit(Vec2 a);

// Support for unordered_map<Vec2>
namespace std
{
	template <>
	struct hash<Vec2>
	{
		size_t operator()(Vec2 v) const
		{
			return static_cast<size_t>(v.x) & (static_cast<size_t>(v.y) << 32);
		}
	};
}

// Support for std::format
// This didn't work - maybe a bug, or need to update visual studio.  Oh well.
/*template<>
struct std::formatter<Vec2>
{
	constexpr auto parse(format_parse_context& ctx)
	{
		return ctx.begin(); //end(ctx);
	}
	auto format(const Vec2& v, format_context& ctx) const
	{
		return std::format_to(ctx.out(),
			"({}, {})", v.x, v.y);
	}
};*/

//------------------------------------------------------------------------------
// Integer Vec3 class

struct Vec3
{
	union
	{
		struct
		{
			int x;
			int y;
			int z;
		};

		int data[3];
	};

	int& operator[] (unsigned int n) { return data[n]; }
	const int& operator[] (unsigned int n) const { return data[n]; }

	Vec2 xy() const { return { x,y }; }
	Vec3 adjusted(Axis a, int n) const;
};

inline bool operator== (Vec3 lhs, Vec3 rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!= (Vec3 lhs, Vec3 rhs)
{
	return !(operator==(lhs, rhs));
}

inline Vec3 operator+ (Vec3 lhs, Vec3 rhs)
{
	return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline Vec3 operator- (Vec3 lhs, Vec3 rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline Vec3 operator* (int lhs, Vec3 rhs)
{
	return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
}

inline Vec3 operator* (Vec3 lhs, int rhs)
{
	return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

inline Vec3 operator/ (Vec3 lhs, int rhs)
{
	return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

inline Vec3 const& operator+= (Vec3& lhs, Vec3 rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

inline Vec3 const& operator-= (Vec3& lhs, Vec3 rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

// no operator*= for int *= Vec3

inline Vec3 const& operator*= (Vec3& lhs, Vec3 rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	return lhs;
}

inline Vec3 const& operator/= (Vec3& lhs, Vec3 rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;
	lhs.z /= rhs.z;
	return lhs;
}

// Conversion from vec2
inline Vec3 Vec2::xy0() const
{
	return {x, y, 0};
}

inline Vec3 Vec2::xyz(int z) const
{
	return {x, y, z};
}

// Component-wise vector min and max
inline Vec3 componentwise_min(Vec3 a, Vec3 b);
inline Vec3 componentwise_max(Vec3 a, Vec3 b);

// Calls rounded_range on the xy components of the vectors.
inline bool range_2d(Vec3 p0, Vec3 p1, int max_range)
{
	return rounded_range(p0.xy(), p1.xy(), max_range);
}

// Get euclidean distance on the xy components of the vectors.
inline float euclid_2d(Vec3 p0, Vec3 p1)
{
	return euclid(p0.xy(), p1.xy());
}

// Get manhattan distance on the xy components of the vectors.
inline int manhattan_2d(Vec3 p0, Vec3 p1)
{
	return manhattan(p0.xy(), p1.xy());
}

// Get chessboard distance on the xy components of the vectors.
inline int chessboard_2d(Vec3 p0, Vec3 p1)
{
	return chessboard(p0.xy(), p1.xy());
}

// For the rare case that you really want to do these operations in 3D space.
int square_dist_3d(Vec3 p0, Vec3 p1);
bool strict_range_3d(Vec3 p0, Vec3 p1, int range);
bool range_3d(Vec3 p0, Vec3 p1, float range);
float euclid_3d(Vec3 p0, Vec3 p1);
int manhattan_3d(Vec3 p0, Vec3 p1);
int chessboard_3d(Vec3 p0, Vec3 p1);

// Support for unordered_map<Vec3>
namespace std
{
	template <>
	struct hash<Vec3>
	{
		size_t operator()(Vec3 v) const
		{
			// I doubt this is a really good hash function, but what I've done is
			// chop the z in half and xor the halves with the upper halves of x and y.
			int const z0 = (0x0000ffff & v.z) << 16;
			int const z1 = (0xffff0000 & v.z);
			int const xz0 = v.x ^ z0;
			int const yz1 = v.y ^ z1;

			return static_cast<size_t>(xz0) & (static_cast<size_t>(yz1) << 32);
		}
	};
}

//------------------------------------------------------------------------------
// Interval class (1D box?)

struct Interval
{
	int min = 0;
	int max = 0;

	int length() { return max - min; }
	bool empty() { return length() <= 0; }

	bool overlaps(Interval other) const;
	Interval overlap(Interval other) const;

	static Interval spanning(int a, int b);
};

//------------------------------------------------------------------------------
// Integer Box 2D class

struct Box2
{
	Box2() = default;
	Box2(Vec2 m, Vec2 s) : min(m), size(s) {}
	Box2(int x, int y, int w, int h) : min{x,y}, size{w,h} {}

	Box2(Interval xi, Interval yi) :
		min{xi.min, yi.min},
		size{xi.max - xi.min, yi.max - yi.min}
	{}

	static Box2 spanning(Vec2 p0, Vec2 p1)
	{
		Box2 b;
		b.min = componentwise_min(p0, p1);
		b.size = componentwise_max(p0, p1) - b.min + Vec2{1,1};
		return b;
	}
	static Box2 around_tile(Vec2 centre, int border)
	{
		return Box2{
			centre - Vec2{border,border},
			{2*border+1,2*border+1}
		};
	}

	Vec2 min = {0,0};
	Vec2 size = {0,0};

	// Note that the cell at max is NOT occupied by the box
	Vec2 max () const { return min + size; }
	int max(Axis a) const { return min[a] + size [a]; }

	// The last cell included inside the box.
	Vec2 inner_max() const { return max() - Vec2{1, 1}; }
	int inner_max(Axis a) const { return max(a) - 1; }

	Vec2 centre () const { return min + size / 2; }
	int centre(Axis a) const { return min[a] + size [a] / 2; }

	int area () const { return size.x * size.y; }

	bool contains (Vec2 v) const;
	bool contains (Box2 other) const;

	bool intersects (Box2 other) const;
	bool intersects_or_adjacent (Box2 other) const;
	Box2 intersection (Box2 other) const;

	// Returns c_CompassNorth if other is adjacent to this box on the north side, etc.
	// Return c_CompassInvalid if boxes are not touching.
	CompassDirection adjacent_edge (Box2 other) const;

	Interval interval_on_axis (Axis a) const;
	bool overlaps_on_axis(Box2 other, Axis a) const;
	Interval overlap_on_axis(Box2 other, Axis a) const;

	Box2 minus_border(int border_size) const;
	Box2 plus_border(int border_size) const;
	Vec2 snap_to_inner_border(Vec2 v, CompassDirection edge) const;
	Vec2 snap_to_outer_border(Vec2 v, CompassDirection edge) const;
	Box2 inner_border_box(CompassDirection edge) const;
	Box2 outer_border_box(CompassDirection edge) const;
};

//------------------------------------------------------------------------------
// Integer Box 3D class

struct Box3
{
	Box3() = default;
	Box3(Vec3 m, Vec3 s) : min(m), size(s) {}
	Box3(int x, int y, int z, int sx, int sy, int sz) :
		min{x,y,z}, size{sx,sy,sz} {}

	Vec3 min = {0,0,0};
	Vec3 size = { 0,0,0 };

	// Note that the cell at max is NOT occupied by the box
	Vec3 max() const { return min + size; }
	int max(Axis a) const { return min[a] + size[a]; }

	// The last cell included inside the box.
	Vec3 inner_max() const { return max() - Vec3{1, 1, 1}; }
	int inner_max(Axis a) const { return max(a) - 1; }

	Vec3 centre () const { return min + size / 2; }
	int centre(Axis a) const { return min[a] + size [a] / 2; }

	bool contains(Vec3 v) const;
	bool intersects(Box3 other) const;
	bool contains(Box3 other) const;
	int area() const { return size.x * size.y; }

	Box3 intersection(Box3 other) const;
};

//------------------------------------------------------------------------------

// Class to iterate over compass directions.
class CompassItr
{
public:
	CompassItr(bool include_no_move) : include(include_no_move), dir(c_CompassEast) {}

	void advance () { dir = (CompassDirection)(dir + 1); }
	bool finished () const { return dir >= (include ? c_CompassCount : c_CompassNoMove); }
	CompassDirection get () const { return dir; }
	Vec2 get_vec2 () const { return c_Compass[dir]; }
	Vec3 get_vec3 () const { return c_Compass[dir].xy0(); }

	// iterator style functions
	explicit operator bool () const { return !finished(); }
	CompassDirection operator* () const { return get(); }
	CompassDirection operator++() { advance(); return dir; }
	bool operator== (CompassItr rhs) const { return dir == rhs.dir; } 
	bool operator!= (CompassItr rhs) const { return dir != rhs.dir; } 
	bool operator== (CompassDirection rhs) const { return dir == rhs; } 
	bool operator!= (CompassDirection rhs) const { return dir != rhs; } 
	// post-increment not provided to avoid accidental copy

private:
	bool include;
	CompassDirection dir;
};

//------------------------------------------------------------------------------

// Class to iterate over all points on rectangle
class BoxItr
{
public:
	BoxItr(Box2 b);

	Vec2 current;
	void advance ();
	bool finished () const;

	// iterator functions
	Vec2 const & operator*() const { return current; }
	Vec2 const * operator->() const { return &current; }
	Vec2 const & operator++() { advance(); return current; }
	bool operator!= (BoxItr rhs) const { return current != rhs.current; } 
	explicit operator bool() const { return !finished(); }
	// post-increment not provided to avoid accidental copy

private:
	Box2 box;
};

// range-based for loop on box
BoxItr begin(Box2 b);
BoxItr end(Box2 b);
