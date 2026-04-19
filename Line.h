#pragma once

#include "Geometry.h"
#include "Types.h"

//------------------------------------------------------------------------------
// Line-drawing methods
// Used for line of sight and beam trajectories.
//------------------------------------------------------------------------------

namespace LineCache
{
	// Constant used to indicate a trajectory up/down stairs rather than in a
	// horizontal direction.
	int constexpr c_StairsLine = -2;

	void init();

	std::vector<int> const& get_lines(Vec2 relative_pos);
	std::vector<int> const& get_lines(Vec2 start, Vec2 end);
	int get_num();
	Vec2 read_line(int line_id, int step);

	// The cache also includes some special asymmetric lines which should only be
	// used when tracing to a solid tile.  May return c_Invalid if none exists.
	int get_asymmetric_line(Vec2 relative_pos);
	int get_asymmetric_line(Vec2 start, Vec2 end);

	// Iterator using a cached line.
	class Itr
	{
	public:
		Itr(Vec2 start_pos, int line_id);

		int steps_left() const;
		bool finished() const { return steps_left() < 0; }
		void advance();

		// As advance, but if end is reached, reset the start position to the end
		// and keep going.  This means the line will never be finished.
		void advance_and_loop();

		// iterator-style functions
		operator bool() const { return !finished(); }
		Vec2 operator*() const { return current; }
		Vec2 const* operator->() const { return &current; }
		Vec2 operator++() { advance(); return current; }
		// post-increment not provided to avoid accidental copy

	protected:
		Vec2 start;
		Vec2 current;
		int id;
		int step;
	};

	// Iterator which stores a 3D point for convenience.
	// Logic is passthrough to 2D iterator.
	class Itr3D
	{
	public:
		Itr3D(Vec3 start_pos, int line_id) :
			itr({ start_pos.x, start_pos.y }, line_id),
			z(start_pos.z)
		{}

		int const steps_left() const { return itr.steps_left(); }
		bool const finished() const { return itr.finished(); }
		void advance() { itr.advance(); }
		void advance_and_loop() { itr.advance_and_loop(); }

		// iterator-style functions
		operator bool() const { return !finished(); }
		Vec3 operator*() const { return {itr->x, itr->y, z}; }
		Vec3 operator++() { advance(); return operator*(); }
		// post-increment not provided to avoid accidental copy
		// can't easily provide ->, I fear.

		LineCache::Itr to_2d() const { return itr; }

	protected:
		LineCache::Itr itr;
		int z;
	};
}

// Line iterator based on Bresenham/Thong algorithm.
// Usage example:
//	for(LineItr itr(p0, p1, LineItr::Thong); itr(); ++itr)
//	{
//		draw_point(*itr);
//	}
class LineItr
{
public:
	// How to resolve cases where point falls exactly between two squares.
	// For example, in a line from A->B below, the line could go through either 1 or 2.
	//
	// A 1
	//   2 B
	//
	enum RoundMode
	{
		// Favour diagonal move (position 2 in the diagram above).
		RoundUp,

		// Favour straight move (position 1 in the diagram above).
		RoundDown,

		// Thong's algorithm: Rounds up when going up, or down when going down.
		// This means the line from A->B is the same as the line from B->A,
		// but lines to symmetrical relative positions are not always symmetrical.
		Thong
	};

	LineItr(Vec2 const& start, Vec2 const& end, RoundMode mode);

	Vec2 current;
	int steps_left;

	void advance();

	// Have we gone past the end point of the line?
	// Note: It is safe to keep advancing and using the line past the end point.
	//   It will continue along the same trajectory.
	bool finished() const { return steps_left < 0; }

	// iterator-style functions
	operator bool() { return !finished(); }
	Vec2 const& operator*() const { return current; }
	Vec2 const* operator->() const { return &current; }
	Vec2 const& operator++() { advance(); return current; }
	// post-increment not provided to avoid accidental copy

private:

	int dist_long;
	int offset;
	Vec2 step[2];
	int d_offset[2];
	int modifier;
};
