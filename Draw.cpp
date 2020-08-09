#include "Draw.h"

#include "Creature.h"
#include "Global.h"
#include "Map.h"
#include "Player.h"
#include "Target.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>

static int constexpr ANIMATION_STEP_MS = 25;

static std::list<std::string> game_messages;
static int constexpr MAX_GAME_MESSAGES = 100;

static std::string spell_preview_string;

static int constexpr TILE_WIDTH_FACTOR = 2;

// ------------------------------------------------------------------------------------------------
// TerminalLayer helper class

class TerminalLayer
{
public:
	enum Layer : byte
	{
		Base = 0,
		Animation
	};

	TerminalLayer (Layer layer);
	~TerminalLayer ();
private:
	int old_layer;
};

TerminalLayer::TerminalLayer (Layer layer)
{
	old_layer = terminal_state(TK_LAYER);
	terminal_layer(layer);
}

TerminalLayer::~TerminalLayer ()
{
	terminal_layer(old_layer);
}

// ------------------------------------------------------------------------------------------------
// Drawing functions

void init_draw ()
{
	game_messages.clear();
	spell_preview_string = "__";
}

DrawView get_draw_view ()
{
	int constexpr view_size = 31;
	Box viewport = make_box(0,0,view_size, view_size);

	// centre the view on the player
	int constexpr half_size = view_size / 2;
	Vec2 constexpr half_vec {half_size, half_size};
	Vec2 start = Player::pos() - half_vec;

	return DrawView{viewport, start};
}

void draw_tile (int code, Vec2 const & global_pos, DrawView const & view,
	char const * const colour)
{
	terminal_color(colour);

	Vec2 viewport_pos = global_pos + view.viewport.min - view.start;

	terminal_put(viewport_pos.x * TILE_WIDTH_FACTOR, viewport_pos.y, code);
}

void draw_tile_bg (int code, Vec2 const & global_pos, DrawView const & view,
	char const * const colour, char const * const bg_colour)
{
	terminal_bkcolor(bg_colour);
	draw_tile(code, global_pos, view, colour);
	terminal_bkcolor("black");
}

void draw_tile_temp (int code, Vec2 const & global_pos, DrawView const & view,
	char const * const colour)
{
	TerminalLayer layer_scope(TerminalLayer::Animation);
	draw_tile(code, global_pos, view, colour);
	terminal_refresh();
	terminal_delay(ANIMATION_STEP_MS);

	// remove animation after
	draw_tile(' ', global_pos, view, "white");
}

void print_in_box (Box const & box, char const * const str, int align)
{
	terminal_print_ext(box.min.x, box.min.y, box.size.x, box.size.y, align, str);
}

void add_game_message(std::string && message)
{
	game_messages.push_back(message);

	// dump old messages...
	if (game_messages.size() > MAX_GAME_MESSAGES)
	{
		game_messages.pop_front();
	}
}

void run_game_message(std::string && message)
{
	game_messages.back().append(message);
}

void print_game_messages(Box const & box)
{
	// We want to print as many messages as we can within the box available.
	// To do this we will concatenate the messages to be printed into a single string.
	// Start with most recent message and keep adding more at the beginning as long as it will fit.

	int lines_left = box.size.y;
	
	std::string combined_message;
	for (auto itr = game_messages.rbegin();
		 itr != game_messages.rend();
		 ++ itr)
	{
		dimensions_t next_size = terminal_measure_ext(box.size.x, box.size.y, itr->c_str());
		lines_left -= next_size.height;
		if (lines_left < 0)
		{
			break;
		}
		else
		{
			combined_message = *itr + "\n" + combined_message;
		}
	};

	print_in_box(box, combined_message.c_str(), TK_ALIGN_LEFT);
}

void set_spell_preview_string(std::string preview)
{
	spell_preview_string = preview;
}

void update_screen ()
{
	terminal_clear();

	DrawView view = get_draw_view();

	terminal_font("tile");
	g_map().draw(view);
	draw_creature(Creature::Player, view);
	//g_player().draw(view);
	draw_visible_creatures(view);

	// restore default font for printing text
	terminal_font("");

	// player stat areas
	Box player_stat_area = make_box(63, 1, 60, 6);
	print_player_stats(player_stat_area);

	// creature stat areas
	int creature_lines = num_lines_for_visible_creature_stats();
	Box creature_stat_area = make_box(63, 8, 60, creature_lines);
	print_visible_creature_stats(creature_stat_area);

	// game message area
	int message_top = 8 + creature_lines;
	int message_lines = 22 - creature_lines;
	Box message_area = make_box(63,message_top,60,message_lines);
	print_game_messages(message_area);

	// spells area
	Box spell_area = make_box(93, 1, 27, 30);
	print_in_box(spell_area, spell_preview_string.c_str());
/*	print_in_box(spell_area,
		"Spells:\n"
		"RL  Relashio\n"
		"FP  Flipendo\n"
		"MW  Mimblewimble\n"
		"RS  Rictusempra\n"
	);*/
}

void draw_screen ()
{
	update_screen();
	terminal_refresh();
}

int num_lines_for_visible_creature_stats ()
{
	int desired = static_cast<int>(g_visible_creatures.size() * 3);
	int constexpr max_lines = 15;
	return std::min(desired, max_lines);
}

static void format_creature_stats (std::stringstream & ss, int ci)
{
	if (creature_is_targeted(ci))
	{
		ss << "[bkcolor=darkest red]";
	}
	ss << std::left << std::setw(16) << creature_name(ci);
	if (creature_is_targeted(ci))
	{
		ss << "[/bkcolor]";
	}

	ss << std::right << std::setw(3) << creature_hp(ci);
	ss << " / ";
	ss << std::left << std::setw(3) << creature_max_hp(ci);
	ss << std::endl;
	ss << "[color=lighter yellow]";
	ss << creature_status_string(ci);
	ss << "[/color]";
}

void print_player_stats (Box draw_area)
{
	std::stringstream ss;

	ss << "XP     " << 125 << std::endl; // todo
	ss << "Magic  " << creature_skill_magic(Creature::Player)  << std::endl;
	ss << "Level  " << 4 << std::endl; // todo
	ss << std::endl;

	format_creature_stats(ss, Creature::Player);

	std::string player_status_string = ss.str();
	print_in_box(draw_area, player_status_string.c_str());
}

void print_visible_creature_stats (Box draw_area)
{
	std::stringstream ss;

	for (int vci = 0; vci < g_visible_creatures.size(); vci++)
	{
		int ci = g_visible_creatures[vci];

		format_creature_stats(ss, ci);

		if (vci < g_visible_creatures.size() - 1)
		{
			ss << std::endl << std::endl;
		}
	}

	std::string creature_status_string = ss.str();
	print_in_box(draw_area, creature_status_string.c_str());
}
