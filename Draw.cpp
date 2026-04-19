#include "Draw.h"

#include "Colour.h"
#include "Config.h"
#include "Creature.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Input.h"
#include "Map.h"
#include "Math.h"
#include "Menu.h"
#include "Player.h"
#include "Stairs.h"
#include "Crosshair.h"
#include "VectorUtil.h"
#include "Visibility.h"
#include "World.h"

#include <algorithm>
#include <format>
#include <iomanip>
#include <sstream>

namespace Draw
{

// Circular array for game messages.
std::vector<GameMessage> s_game_messages;
int constexpr c_MaxGameMessages = 400;
int s_next_message_id = 0;
int s_message_indent = 0;

static int constexpr c_AnimationStepMs = 25;
static int constexpr c_TileWidthFactor = 2;

static int constexpr c_StatAreaWidth = 57;
static int constexpr c_SpellAreaWidth = 27;

bool s_los_cheat = false;

View s_view = View{};

// ------------------------------------------------------------------------------------------------
// TerminalLayer helper class

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
	s_view.peek_tiles.reserve(1);
}

void clear ()
{
	s_game_messages.clear();
	s_next_message_id = 0;
	s_message_indent = 0;
}

bool View::contains_global_pos(Vec3 const& global_pos) const
{
	return view_area().contains(global_pos.xy())
		&& (global_pos.z == z || Util::Contains(peek_tiles, global_pos));
}

int View::get_z(Vec2 pos2) const
{
	for (Vec3 peek : peek_tiles)
	{
		if (peek.xy() == pos2)
		{
			return peek.z;
		}
	}

	return z;
}

Vec3 View::mouse_to_global_pos() const
{
	Vec2 const mouse_pos {
		(terminal_state(TK_MOUSE_X) - viewport.min.x) / c_TileWidthFactor,	// integer division
		terminal_state(TK_MOUSE_Y) - viewport.min.y
	};

	Vec2 const pos2 = start + mouse_pos;

	return pos2.xyz(get_z(pos2));
}

void update_view (Box2 viewport)
{
	s_view.viewport = viewport;

	// Centre the view on the player
	Vec2 const half_vec{viewport.size.x/2, viewport.size.y/2};
	Vec3 const viewer = Player::pos();
	s_view.start = viewer.xy() - half_vec;
	s_view.z = viewer.z;

	s_view.ignore_visibility = s_los_cheat;

	// Add stairs exception
	s_view.peek_tiles.clear();
	Box2 const view_area = s_view.view_area();
	World const& world = World::read();

	for (int i = 0; i < world.num_maps(); ++i)
	{
		Map const& map = world.read_map(i);
		if (map.get_z() == s_view.z &&
			map.get_box().intersects(s_view.view_area()))
		{
			for (Stairs::Pair pair : map.get_stairs_map())
			{
				Vec3 this_end = pair.first.xyz(s_view.z);
				Vec3 other_end = this_end + Stairs::relative_move(pair.second);
				if (view_area.contains(other_end.xy()) &&
					world.get_visibility(this_end) != Visibility::Hidden)
				{
					s_view.peek_tiles.push_back(other_end);
				}
			}
		}
	}
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
	draw_tile(' ', global_pos, view, cstr_White);
}

void anim_delay()
{
	terminal_delay(c_AnimationStepMs);
}

void print_in_box (Box2 const & box, char const * const str, int align)
{
	terminal_print_ext(box.min.x, box.min.y, box.size.x, box.size.y, align, str);
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

bool los_cheat_enabled()
{
	return s_los_cheat;
}

//-------------------------------------------------------------------------------------------------
// Game message system

void add_message_indent()
{
	++s_message_indent;
}

void reduce_message_indent()
{
	--s_message_indent;
}

void add_message(std::string && message, char const* colour)
{
	if (s_message_indent > 0)
	{
		message = message.insert(0, s_message_indent, ' ');
	}

	if (Util::Size(s_game_messages) < c_MaxGameMessages)
	{
		s_game_messages.push_back({
			.text = message,
			.colour = colour,
			.turn_number = Game::get_turn_number()
		});
	}
	else
	{
		// It's a circular array.
		s_game_messages[s_next_message_id] = {
			.text = message,
			.colour = colour,
			.turn_number = Game::get_turn_number()
		};
		s_next_message_id = (s_next_message_id + 1) % c_MaxGameMessages;
	}
}

void creature_message(Creature::Handle creature, std::string&& message,	char const* colour)
{
	if (creature.visible())
	{
		add_message(std::move(message), colour);
	}
}

void pos_message(Vec3 pos, std::string&& message, char const* colour)
{
	if (World::read().is_visible(pos))
	{
		add_message(std::move(message), colour);
	}
}

void print_messages(Box2 const & box)
{
	int const total_messages = get_num_recent_messages();
	if (total_messages == 0)
	{
		return;
	}
	int const newest_time = get_recent_message(0).turn_number;

	// We want to print as many messages as we can within the box available.
	// We will start with the most recent message and keep measuring messages and adding
	// their height until we run out of messages or find one that won't fit.
	int num_to_print = 0;
	int combined_height = 0;
	while (num_to_print < total_messages && combined_height < box.size.y)
	{
		GameMessage const& message = get_recent_message(num_to_print);
		int const next_height = terminal_measure_ext(box.size.x, box.size.y,
			message.text.c_str()).height;
		combined_height += next_height;
		if (combined_height <= box.size.y)
		{
			++num_to_print;
		}
	}

	// Now that we know how many to print, print them each in reverse order (from top to bottom).
	int print_y = box.min.y;
	for (int i = num_to_print - 1; i >= 0; --i)
	{
		GameMessage const& message = get_recent_message(i);

		if (message.turn_number == newest_time)
		{
			if (message.colour == nullptr)
			{
				terminal_color(cstr_White);
			}
			else
			{
				terminal_color(message.colour);
			}
		}
		else
		{
			terminal_color(cstr_Grey);
		}

		dimensions_t const dim = terminal_print_ext(box.min.x, print_y, box.size.x, box.size.y,
			TK_ALIGN_LEFT, message.text.c_str());
		print_y += dim.height;
	}

	// Restore default font.
	terminal_color(cstr_White);
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

// ------------------------------------------------------------------------------------------------
// Internal function implementations

void update_screen()
{
	int const c_StatAreaLeft = Config::get_width() - c_StatAreaWidth;
	int const c_SpellAreaLeft = Config::get_width() - c_SpellAreaWidth;

	int const c_ViewWidth = (c_StatAreaLeft - 1) / c_TileWidthFactor; // View uses wide tiles
	int const c_ViewHeight = Config::get_height() - 1;

	update_view(Box2{0, 1, c_ViewWidth, c_ViewHeight});
	Draw::View const& view = get_view();

	terminal_font("tile");
	World::read().draw(view);

	Gingerbread::edit_player_stats().colour = get_hp_colour(Player::handle());
	draw_creature(Creature::Player, view);

	Creature::draw_visible_creatures(view);
	Crosshair::draw(view);

	// restore default font for printing text
	terminal_font("");
	terminal_color(cstr_White);

	// map name
	char const* map_name = World::read().find_map_name(Player::pos());
	terminal_print_ext(0, 0, c_StatAreaLeft - 1, 1, TK_ALIGN_CENTER, map_name);

	// player stat areas
	Box2 const player_stat_area = Box2(c_StatAreaLeft, 1, c_StatAreaWidth, 6);
	print_player_stats(player_stat_area);

	// creature stat areas
	int const creature_lines = num_lines_for_visible_creature_stats();
	Box2 const creature_stat_area = Box2(c_StatAreaLeft, 8, c_StatAreaWidth, creature_lines);
	print_visible_creature_stats(creature_stat_area);

	// game message area
	int const message_top = 8 + creature_lines;
	int const message_lines = Config::get_height() - message_top;
	Box2 message_area = Box2(c_StatAreaLeft, message_top, c_StatAreaWidth, message_lines);
	print_messages(message_area);

	// spells area
	Box2 const spell_area = Box2(c_SpellAreaLeft, 1, c_SpellAreaWidth, 30);
	std::string const spell_preview = Input::get_spell_preview_string();
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
		return cstr_White;
	}
	else if (hp_percent >= 0.499f)
	{
		return cstr_LightYellow;
	}
	else if (hp_percent >= 0.249f)
	{
		return cstr_LightOrange;
	}
	else
	{
		return cstr_LightRed;
	}
}

void format_creature_stats(std::stringstream& ss, Creature::Handle creature)
{
	if (Crosshair::is_target(creature))
	{
		ss << "[bkcolor=" << Crosshair::colour() << "]";
	}
	ss << std::left << std::setw(16) << creature.short_name();
	if (Crosshair::is_target(creature))
	{
		ss << "[/bkcolor]";
	}

	ss << "[color=" << get_hp_colour(creature) << "]";
	ss << std::right << std::setw(3) << creature.hp();
	ss << " / ";
	ss << std::left << std::setw(3) << creature.max_hp();
	ss << "[/color]";

	ss << std::endl;
	ss << creature.status_string();
}

void print_player_stats(Box2 draw_area)
{
	std::stringstream ss;

	int const sugar_int = Math::RoundToInt(Player::get_sugar());
	char const* sugar_colour = Player::get_sugar_colour();

	ss << "Level  " << Player::current_level() << std::endl;
	ss << "XP     " << Player::current_xp() << " / "
	   << Player::next_xp_threshold() << std::endl;
	ss << "Sugar  [color=" << sugar_colour << "]"
		<< sugar_int << " %" << "[/color]" << std::endl;
	//ss << "Magic  " << Player::handle().skill_magic() << std::endl;
	ss << std::endl;

	format_creature_stats(ss, Creature::Player);

	std::string player_status_string = ss.str();
	print_in_box(draw_area, player_status_string.c_str());
}

void print_visible_creature_stats(Box2 draw_area)
{
	std::stringstream ss;

	Creature::HandleList const& visible_creatures = Creature::get_visible_creatures();
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

} // namespace Draw
