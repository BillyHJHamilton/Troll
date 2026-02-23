#pragma once

#include "Types.h"
#include "Geometry.h"

#include <optional>

namespace Target
{
	void init();
	void clear();
	void update();
	void cycle();
	void move(Vec2 dir);

	bool is_valid();
	bool is_target(Creature::Handle creature);
	bool is_target(Vec3 const& global_pos);
	std::optional<Vec3> get_pos();

	char const* colour();
	char const* colour(Visibility visibility, bool is_wall);
	void draw(Draw::View view);
}
