#pragma once

#include "Geometry.h"
#include "Line.h"

// Version of line itr that tracks 3D position in world space.
// It will move up and down staircases it encounters.
// Not sure this is useful tbh.  May want to special-case it instead.
class WorldLineItr
{
public:
	WorldLineItr(Vec3 start_pos, int line_id) :
		itr({ start_pos.x, start_pos.y }, line_id),
		z(start_pos.z)
	{}

	int steps_left() const { return itr.steps_left(); }
	bool finished() const { return itr.finished(); }
	void advance();
	void advance_and_loop();
	Vec3 current() { return {itr->x, itr->y, z}; }

	// iterator-style functions
	operator bool() { return !finished(); }
	Vec3 operator*() { return current(); }
	Vec3 operator++() { advance(); return current(); }
	// post-increment not provided to avoid accidental copy
	// can't easily provide ->, I fear.

	LineCache::Itr to_line_itr() const { return itr; }

protected:
	LineCache::Itr itr;
	int z;
};
