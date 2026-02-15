#include "Draw.h"

#include "Creature.h"
#include "Game.h"
#include "Input.h"
#include "Menu.h"
#include "Player.h"
#include "Stairs.h"
#include "Target.h"
#include "VectorUtil.h"
#include "World.h"

#include <algorithm>
#include <format>
#include <iomanip>
#include <sstream>

namespace Draw
{

// Circular array.
std::vector<GameMessage> s_game_messages;
int constexpr c_MaxGameMessages = 100;
int s_next_message_id = 0;

//static std::list<GameMessage> s_game_messages;

static int constexpr c_AnimationStepMs = 25;
static int constexpr c_TileWidthFactor = 2;

bool s_los_cheat = false;

View s_view = View{};

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
void print_player_stats(Box2 draw_area);
void print_visible_creature_stats(Box2 draw_area);

// ------------------------------------------------------------------------------------------------
// Draw interface functions

void init ()
{
	s_game_messages.reserve(c_MaxGameMessages);
}

void clear ()
{
	s_game_messages.clear();
	s_next_message_id = 0;
}

bool View::contains_global_pos(Vec3 const& global_pos) const
{
	return view_area().contains(global_pos.xy())
		&& (global_pos.z == z || Util::Contains(peek_tiles, global_pos));
}

void update_view ()
{
	int constexpr view_size = 31;
	Box2 viewport = Box2(0,0,view_size, view_size);

	// Centre the view on the player
	int constexpr half_size = view_size / 2;
	Vec2 constexpr half_vec {half_size, half_size};
	Vec3 const viewer = Player::pos();
	Vec2 const start = viewer.xy() - half_vec;

	// Add stairs exception
	World const& world = World::read();
	Stairs::Direction dir = world.get_stairs(viewer);
	std::vector<Vec3> peek_tiles;
	if (dir != Stairs::None)
	{
		Vec3 const stairs_pos = viewer + Stairs::relative_move(dir);
		peek_tiles.push_back(stairs_pos);
	}

	s_view =
	{
		viewport,
		start,
		viewer.z,
		s_los_cheat, // ignore visibility
		peek_tiles
	};
}

View const& get_view ()
{
	return s_view;
}

void draw_tile (int code, Vec2 const & global_pos, Draw::View const & view,
	char const * const colour)
{
	terminal_color(colour);

	Vec2 viewport_pos = global_pos + view.viewport.min - view.start;

	terminal_put(viewport_pos.x * c_TileWidthFactor, viewport_pos.y, code);
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
	terminal_delay(c_AnimationStepMs);

	// remove animation after
	draw_tile(' ', global_pos, view, "white");
}

void print_in_box (Box2 const & box, char const * const str, int align)
{
	terminal_print_ext(box.min.x, box.min.y, box.size.x, box.size.y, align, str);
}

void add_message(std::string && message)
{
	if (Util::Size(s_game_messages) < c_MaxGameMessages)
	{
		s_game_messages.push_back({Game::get_turn_number(), message});
	}
	else
	{
		// It's a circular array.
		s_game_messages[s_next_message_id] = {Game::get_turn_number(), message};
		s_next_message_id = (s_next_message_id + 1) % c_MaxGameMessages;
	}
}

void creature_message(Creature::Handle creature, std::string&& message)
{
	if (creature.visible())
	{
		add_message(std::move(message));
	}
}

void pos_message(Vec3 pos, std::string&& message)
{
	if (World::read().is_visible(pos))
	{
		add_message(std::move(message));
	}
}

void print_messages(Box2 const & box)
{
	// We want to print as many messages as we can within the box available.
	// To do this we will concatenate the messages to be printed into a single string.
	// Start with most recent message and keep adding more at the beginning as long as it will fit.

	int lines_left = box.size.y;

	if (!s_game_messages.empty())
	{
		int const num_messages = get_num_recent_messages();
		int const newest_time = get_recent_message(0).turn_number;

		std::string combined_message;
		for (int i = 0; i < num_messages; ++i)
		{
			GameMessage& message = get_recent_message(i);
			dimensions_t next_size = terminal_measure_ext(box.size.x, box.size.y,
				message.text.c_str());
			lines_left -= next_size.height;
			if (lines_left < 0)
			{
				break;
			}
			else
			{
				if (message.turn_number < newest_time)
				{
					combined_message = std::format("[color=grey]{}[/color]\n", message.text)
						+ combined_message;
				}
				else
				{
					combined_message = message.text + "\n" + combined_message;
				}
				
				// Printing turn numbers - test only
				// combined_message = "(" + std::to_string(itr->turn_number) + ") " + combined_message;
			}
		}

		print_in_box(box, combined_message.c_str(), TK_ALIGN_LEFT);
	}
}

int get_num_recent_messages()
{
	return Util::Size(s_game_messages);
}

GameMessage& get_recent_message(int num_back)
{
	if (Util::Size(s_game_messages) < c_MaxGameMessages)
	{
		int const index = Util::LastIndex(s_game_messages) - num_back;
		return s_game_messages.at(index);
	}
	else
	{
		int const index = (c_MaxGameMessages + s_next_message_id - 1 - num_back)
			% c_MaxGameMessages;
		return s_game_messages.at(index);
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

void toggle_los_cheat()
{
	s_los_cheat = 1 - s_los_cheat;
}

// ------------------------------------------------------------------------------------------------
// Internal function implementations

void update_screen()
{
	update_view();
	Draw::View const& view = get_view();

	terminal_font("tile");
	World::read().draw(view);
	draw_creature(Creature::Player, view);
	Creature::draw_visible_creatures(view);

	// LINE DEBUG
	//std::optional<Vec2> target = Target::get_pos();
	//if (target.has_value())
	//{
	//	for (LineItr itr = LineItr(Player::pos(), *target); itr; ++itr)
	//	{
	//		draw_tile('X', *itr, view, "yellow");
	//	}
	//}

	// restore default font for printing text
	terminal_font("");
	terminal_color("white");

	// player stat areas
	Box2 player_stat_area = Box2(63, 1, 60, 6);
	print_player_stats(player_stat_area);

	// creature stat areas
	int creature_lines = num_lines_for_visible_creature_stats();
	Box2 creature_stat_area = Box2(63, 8, 60, creature_lines);
	print_visible_creature_stats(creature_stat_area);

	// game message area
	int message_top = 8 + creature_lines;
	int message_lines = 22 - creature_lines;
	Box2 message_area = Box2(63, message_top, 60, message_lines);
	print_messages(message_area);

	// spells area
	Box2 spell_area = Box2(93, 1, 27, 30);
	std::string spell_preview = Input::get_spell_preview_string();
	print_in_box(spell_area, spell_preview.c_str());
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
		ss << "[bkcolor=" << c_target_colour << "]";
	}
	ss << std::left << std::setw(16) << creature.short_name();
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

void print_player_stats(Box2 draw_area)
{
	std::stringstream ss;

	ss << "Level  " << Player::current_level() << std::endl;
	ss << "XP     " << Player::current_xp() << " / "
	   << Player::next_xp_threshold() << std::endl;
	ss << "Magic  " << Player::handle().skill_magic() << std::endl;
	ss << std::endl;

	format_creature_stats(ss, Creature::Player);

	std::string player_status_string = ss.str();
	print_in_box(draw_area, player_status_string.c_str());
}

void print_visible_creature_stats(Box2 draw_area)
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
