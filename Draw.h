#pragma once

#include "BearLibTerminal.h"

#include "Geometry.h"

struct DrawView
{
	Box viewport; // on the screen
	Vec2 start; // upper left corner of draw area in global coords

	Box view_area () const { return {start, viewport.size}; }
	bool contains_global_pos (Vec2 const & global_pos) const
		{ return view_area().contains(global_pos); }
};

void init_draw ();

DrawView get_draw_view ();

void draw_tile (int code, Vec2  const & global_pos, DrawView const & view,
	char const * const colour);
void draw_tile_bg (int code, Vec2  const & global_pos, DrawView const & view,
	char const * const colour, char const * const bg_colour);
void draw_tile_temp (int code, Vec2  const & global_pos, DrawView const & view,
	char const * const colour);

void print_in_box (Box const & box, char const * const str, int align=TK_ALIGN_LEFT);
void add_game_message (std::string && message);
void run_game_message (std::string && message); // adds to most recent message
void print_game_messages (Box const & box);

void update_screen ();
void draw_screen ();
int num_lines_for_visible_creature_stats ();
void print_player_stats (Box draw_area);
void print_visible_creature_stats (Box draw_area);
