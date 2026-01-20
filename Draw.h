#pragma once

#include "BearLibTerminal.h"

#include "Geometry.h"

namespace Draw
{
	struct View
	{
		Box viewport; // on the screen
		Vec2 start; // upper left corner of draw area in global coords

		Box view_area() const { return { start, viewport.size }; }
		bool contains_global_pos(Vec2 const& global_pos) const
		{
			return view_area().contains(global_pos);
		}
	};

	void init();

	View get_view();

	void draw_tile(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour);
	void draw_tile_bg(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour, char const* const bg_colour);
	void draw_tile_temp(int code, Vec2  const& global_pos, Draw::View const& view,
		char const* const colour);

	void print_in_box(Box const& box, char const* const str, int align = TK_ALIGN_LEFT);
	void add_message(std::string&& message);
	void run_message(std::string&& message); // adds to most recent message
	void print_messages(Box const& box);

	void draw_screen();
}
