#include "Draw.h"

#include "Creature.h"
#include "Game.h"
#include "Input.h"
#include "Map.h"
#include "Menu.h"
#include "Player.h"
#include "Target.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>

namespace Draw
{

struct GameMessage
{
	int turn_number;
	std::string text;
};

static std::list<GameMessage> s_game_messages;
static int constexpr MAX_GAME_MESSAGES = 100;

static int constexpr ANIMATION_STEP_MS = 25;
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
// Internal function declarations

void update_screen();
int num_lines_for_visible_creature_stats();
const char* get_hp_colour(Creature::Handle creature);
void format_creature_stats(std::stringstream& ss, Creature::Handle creature);
void print_player_stats(Box draw_area);
void print_visible_creature_stats(Box draw_area);

// ------------------------------------------------------------------------------------------------
// Draw interface functions

void init ()
{
	s_game_messages.clear();
}

View get_view ()
{
	int constexpr view_size = 31;
	Box viewport = make_box(0,0,view_size, view_size);

	// centre the view on the player
	int constexpr half_size = view_size / 2;
	Vec2 constexpr half_vec {half_size, half_size};
	Vec2 start = Player::pos() - half_vec;

	return View{viewport, start};
}

void draw_tile (int code, Vec2 const & global_pos, Draw::View const & view,
	char const * const colour)
{
	terminal_color(colour);

	Vec2 viewport_pos = global_pos + view.viewport.min - view.start;

	terminal_put(viewport_pos.x * TILE_WIDTH_FACTOR, viewport_pos.y, code);
}

void draw_tile_bg (int code, Vec2 const & global_pos, Draw::View const & view,
	char const * const colour, char const * const bg_colour)
{
	terminal_bkcolor(bg_colour);
	draw_tile(code, global_pos, view, colour);
	terminal_bkcolor("black");
}

void draw_tile_temp (int code, Vec2 const & global_pos, Draw::View const & view,
	char const * const colour)
{
	TerminalLayer layer_scope(TerminalLayer::Animation);
	terminal_font("tile");
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

void add_message(std::string && message)
{
	s_game_messages.push_back({Game::get_turn_number(), message});

	// dump old messages...
	if (s_game_messages.size() > MAX_GAME_MESSAGES)
	{
		s_game_messages.pop_front();
	}
}

void run_message(std::string && message)
{
	s_game_messages.back().text.append(message);
}

void print_messages(Box const & box)
{
	// We want to print as many messages as we can within the box available.
	// To do this we will concatenate the messages to be printed into a single string.
	// Start with most recent message and keep adding more at the beginning as long as it will fit.

	int lines_left = box.size.y;

	if (!s_game_messages.empty())
	{
		int newest_time = s_game_messages.back().turn_number;

		std::string combined_message;
		for (auto itr = s_game_messages.rbegin();
			itr != s_game_messages.rend();
			++itr)
		{
			dimensions_t next_size = terminal_measure_ext(box.size.x, box.size.y, itr->text.c_str());
			lines_left -= next_size.height;
			if (lines_left < 0)
			{
				break;
			}
			else
			{
				if (itr->turn_number < newest_time)
				{
					combined_message = "[color=grey]" + itr->text + "[/color]\n" + combined_message;
				}
				else
				{
					combined_message = itr->text + "\n" + combined_message;
				}
			}
		};

		print_in_box(box, combined_message.c_str(), TK_ALIGN_LEFT);
	}
}

void draw_screen ()
{
	terminal_clear();

	if (Game::get_mode() == GameMode::Menu)
	{
		Menu::update_screen();
	}
	else
	{
		update_screen();
	}

	terminal_refresh();
}

// ------------------------------------------------------------------------------------------------
// Internal function implementations

void update_screen()
{
	Draw::View view = get_view();

	terminal_font("tile");
	g_map().draw(view);
	draw_creature(Creature::Player, view);
	//g_player().draw(view);
	Creature::draw_visible_creatures(view);

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
	Box message_area = make_box(63, message_top, 60, message_lines);
	print_messages(message_area);

	// spells area
	Box spell_area = make_box(93, 1, 27, 30);
	std::string spell_preview = Input::get_spell_preview_string();
	print_in_box(spell_area, spell_preview.c_str());
	/*	print_in_box(spell_area,
			"Spells:\n"
			"RL  Relashio\n"
			"FP  Flipendo\n"
			"MW  Mimblewimble\n"
			"RS  Rictusempra\n"
		);*/
}

int num_lines_for_visible_creature_stats()
{
	int desired = static_cast<int>(Creature::get_visible_creatures().size() * 3);
	int constexpr max_lines = 15;
	return std::min(desired, max_lines);
}

const char* get_hp_colour(Creature::Handle creature)
{
	const float hp_percent = creature.hp_percent();

	if (hp_percent > 0.99f)
	{
		return "[color=white]";
	}
	else if (hp_percent >= 0.499f)
	{
		return "[color=light yellow]";
	}
	else if (hp_percent >= 0.249f)
	{
		return "[color=light orange]";
	}
	else
	{
		return "[color=light red]";
	}
}

void format_creature_stats(std::stringstream& ss, Creature::Handle creature)
{
	if (Target::is_target(creature))
	{
		ss << "[bkcolor=darkest red]";
	}
	ss << std::left << std::setw(16) << creature.name();
	if (Target::is_target(creature))
	{
		ss << "[/bkcolor]";
	}

	ss << get_hp_colour(creature);
	ss << std::right << std::setw(3) << creature.hp();
	ss << " / ";
	ss << std::left << std::setw(3) << creature.max_hp();
	ss << "[/color]";

	ss << std::endl;
	ss << "[color=lighter yellow]";
	ss << creature.status_string();
	ss << "[/color]";
}

void print_player_stats(Box draw_area)
{
	std::stringstream ss;

	ss << "XP     " << 125 << std::endl; // todo
	ss << "Magic  " << Player::handle().skill_magic() << std::endl;
	ss << "Level  " << 4 << std::endl; // todo
	ss << std::endl;

	format_creature_stats(ss, Creature::Player);

	std::string player_status_string = ss.str();
	print_in_box(draw_area, player_status_string.c_str());
}

void print_visible_creature_stats(Box draw_area)
{
	std::stringstream ss;

	std::vector<Creature::Handle> visible_creatures = Creature::get_visible_creatures();
	for (int vci = 0; vci < visible_creatures.size(); vci++)
	{
		Creature::Handle ci = visible_creatures[vci];

		format_creature_stats(ss, ci);

		if (vci < visible_creatures.size() - 1)
		{
			ss << std::endl << std::endl;
		}
	}

	std::string creature_status_string = ss.str();
	print_in_box(draw_area, creature_status_string.c_str());
}

}