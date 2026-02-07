#pragma once

#include "BearLibTerminal.h"

#include "Geometry.h"

namespace Draw
{
	struct View
	{
		Box2 viewport; // on the screen
		Vec2 start; // upper left corner of draw area in global coords
		int z; // vertical slice to draw
		bool ignore_visibility;
		std::vector<Vec3> peek_tiles; // used for stairs

		Box2 view_area() const { return { start, viewport.size }; }
		bool contains_global_pos(Vec3 const& global_pos) const;
	};

	void init();
	void clear();

	View get_view();

	void draw_tile(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour);
	void draw_tile_bg(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour, char const* const bg_colour);
	void draw_tile_temp(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour);

	void print_in_box(Box2 const& box, char const* const str, int align = TK_ALIGN_LEFT);
	void add_message(std::string&& message);
	void run_message(std::string&& message); // adds to most recent message

	// Adds a message only if the creature provided is visible.
	void creature_message(Creature::Handle creature, std::string&& message);

	// Adds message only if location is visible.
	void pos_message(Vec3 pos, std::string&& message);

	void print_messages(Box2 const& box);

	void draw_screen();

	void toggle_los_cheat();
}
