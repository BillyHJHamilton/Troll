#pragma once

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
};

inline bool operator== (Vec2 const & lhs, Vec2 const & rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!= (Vec2 const & lhs, Vec2 const & rhs)
{
	return !(operator==(lhs, rhs));
}

inline Vec2 operator+ (Vec2 const & lhs, Vec2 const & rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Vec2 operator- (Vec2 const & lhs, Vec2 const & rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline Vec2 operator* (int lhs, Vec2 const & rhs)
{
	return {lhs * rhs.x, lhs * rhs.y};
}

inline Vec2 operator* (Vec2 const & lhs, int rhs)
{
	return {lhs.x * rhs, lhs.y * rhs};
}

inline Vec2 const & operator+= (Vec2 & lhs, Vec2 const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline Vec2 const & operator-= (Vec2 & lhs, Vec2 const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

// Component-wise vector min and max
inline Vec2 componentwise_min(Vec2 a, Vec2 b);
inline Vec2 componentwise_max(Vec2 a, Vec2 b);

//Vec2 constexpr WALK_VEC [8] = { {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1} };
//enum class WalkDir : int { E, NE, N, NW, W, SW, S, SE };

// returns true if euclidean distance <= range
bool check_within_range(Vec2 p0, Vec2 p1, int max_range);

// don't use this if you could use the above, of course
float euclidean_distance(Vec2 p0, Vec2 p1);

using Axis = int;
Axis constexpr AXIS_X = 0;
Axis constexpr AXIS_Y = 1;
Axis get_other_axis(Axis a) { return 1 - a; }

struct Box
{
	Vec2 min;
	Vec2 size;

	// Note that the cell at max is NOT occupied by the box
	Vec2 max () const { return min + size; }
	int max(Axis a) const { return min[a] + size [a]; }

	// The last cell included inside the box.
	Vec2 inner_max() const { return max() - Vec2{1, 1}; }
	int inner_max(Axis a) const { return max(a) - 1; }

	bool contains (Vec2 const & v) const;
	bool intersects (Box const & other) const;
	bool contains (Box const & other) const;
	int area () const { return size.x * size.y; }

	Box intersection (Box const & other) const;
};

inline Box make_box(int x, int y, int w, int h)
{
	return { {x,y}, {w,h} };
}

//------------------------------------------------------------------------------

// Class to iterate over all points on rectangle
class BoxItr
{
public:
	BoxItr(Box const & b);

	Vec2 current;
	void advance ();
	bool finished () const;

	// iterator functions
	Vec2 const & operator*() const { return current; }
	Vec2 const * operator->() const { return &current; }
	Vec2 const & operator++() { advance(); return current; }
	bool operator!= (BoxItr const & rhs) const { return current != rhs.current; } 
	operator bool() const { return !finished(); }
	// post-increment not provided to avoid accidental copy

private:
	Box const & box;
};

// range-based for loop on box
BoxItr begin(Box const & b);
BoxItr end(Box const & b);

