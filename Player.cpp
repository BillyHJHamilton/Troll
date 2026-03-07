#include "BearLibTerminal.h"
#include "Player.h"

#include "Action.h"
#include "Bot.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Gingerbread.h"
#include "Math.h"
#include "Pathfind.h"
#include "Serialize.h"
#include "Spell.h"
#include "Visibility.h"
#include "World.h"

namespace Player
{

//-------------------------------------------------------------------------------------------------
// Data

Player::Data s_player_data;

Player::Data const & read_data()
{
	return s_player_data;
}

Player::Data & edit_data()
{
	return s_player_data;
}

//-------------------------------------------------------------------------------------------------
// Global interface

void Player::clear()
{
	s_player_data = Player::Data();
}

void Player::Data::serialize(ISerializer& s)
{
	s.srz_string(name);

	// Could be autosaving during a long move, but we don't want to automove on reload.
	// s.srz_value(automove);

	// Don't need to serialize these since we won't save in the middle of a turn.
	assert(!acted);
	assert(!game_over);
	assert(defeated_by == Creature::None);
	
	s.srz_int(level);
	s.srz_int(xp);
}

void serialize(ISerializer& s)
{
	s_player_data.serialize(s);
}

Vec3 pos()
{
	return handle().pos();
}

Creature::Handle Player::handle()
{
	return 0;
}

const std::string& name()
{
	return read_data().name;
}

bool is_automoving()
{
	return read_data().automove_type != AutomoveType::None;
}

void set_name (std::string str)
{
	edit_data().name = str;
}

void start_automove(CompassDirection dir)
{
	edit_data().automove_type = AutomoveType::Compass;
	edit_data().automove_dir = dir;
}

void start_pathfind (Vec3 target)
{
	if (World::read().get_visibility(target) == Visibility::Hidden)
	{
		Draw::add_message("Can't travel to unexplored area.");
		return;
	}
	else if (World::read().is_solid(target))
	{
		Draw::add_message("Can't travel to that square.");
		return;
	}

	bool success = Bot::try_player_pathfind(target);
	if (success)
	{
		edit_data().automove_type = AutomoveType::Path;
	}
	else
	{
		Draw::add_message("No path found.");
	}
}

void auto_collect ()
{
	bool success = Bot::try_player_collect();
	if (success)
	{
		edit_data().automove_type = AutomoveType::Path;
	}
	else
	{
		Draw::add_message("No items found.");
	}
}

void auto_darkness ()
{
	bool success = Bot::try_player_explore();
	if (success)
	{
		edit_data().automove_type = AutomoveType::Path;
	}
	else
	{
		Draw::add_message("This area seems fully explored.");
	}
}

void auto_explore ()
{
	edit_data().automove_type = AutomoveType::Explore;
}

void stop_automove()
{
	edit_data().automove_type = AutomoveType::None;
	edit_data().automove_dir = c_CompassInvalid;
	Bot::clear_player_path();
}

void dispatch_automove()
{
	AutomoveType const type = read_data().automove_type;

	if (type == AutomoveType::Explore &&
		!Bot::has_player_path())
	{
		bool success = Bot::try_player_collect();

		if (!success)
		{
			success = Bot::try_player_explore();
		}

		if (!success)
		{
			Draw::add_message("Done exploring.");
			stop_automove();
			return;
		}
	}

	if (type == AutomoveType::Compass)
	{
		CompassDirection const dir = read_data().automove_dir;
		if (dir == c_CompassNoMove)
		{
			Action::player_rest_step();

			if (!Player::handle().is_hurt())
			{
				Player::stop_automove();
				return;
			}
		}
		else
		{
			// Automove behaviour, inspired by run system in Linley's Dungeon Crawl.
			CompassDirection const clockwise = get_clockwise(dir);
			CompassDirection const counterclockwise = get_counterclockwise(dir);
			Vec3 const p0 = Player::pos();
			Vec3 const p1 = p0 + c_Compass[clockwise].xy0();
			Vec3 const p2 = p0 + c_Compass[counterclockwise].xy0();
			Terrain::Type t0 = World::read().get_terrain(p0);
			Terrain::Type t1 = World::read().get_terrain(p1);
			Terrain::Type t2 = World::read().get_terrain(p2);

			Vec2 relative_move = c_Compass[dir];
			if (Action::is_move_hazardous(Player::handle(), relative_move))
			{
				Draw::add_message("That looks hazardous.");
				Player::stop_automove();
				return;
			}

			bool const moved = Action::player_try_move(c_Compass[dir]);

			if (!moved)
			{
				Player::stop_automove();
				return;
			}
			else
			{
				Vec3 const new_p0 = Player::pos();
				Vec3 const new_p1 = new_p0 + c_Compass[clockwise].xy0();
				Vec3 const new_p2 = new_p0 + c_Compass[counterclockwise].xy0();
				Terrain::Type new_t0 = World::read().get_terrain(new_p0);
				Terrain::Type new_t1 = World::read().get_terrain(new_p1);
				Terrain::Type new_t2 = World::read().get_terrain(new_p2);

				if (t0 != new_t0 || t1 != new_t1 || t2 != new_t2)
				{
					Player::stop_automove();
					return;
				}
			}
		}
	}

	else if (type == AutomoveType::Path || type == AutomoveType::Explore)
	{
		Vec2 move = Bot::pop_player_path();
		if (move == c_Compass[c_CompassNoMove])
		{
			Player::stop_automove();
			return;
		}
		else if (Action::is_move_hazardous(Player::handle(), move))
		{
			Draw::add_message("That looks hazardous.");
			Player::stop_automove();
			return;
		}
		else
		{
			bool moved = Action::player_try_move(move);
			if (!moved)
			{
				Player::stop_automove();
				return;
			}
		}
	}

	if (Creature::has_visible_enemy())
	{
		Player::stop_automove();
		return;
	}
}

bool has_acted ()
{
	return read_data().acted;
}

void set_acted(bool acted)
{
	s_player_data.acted = acted;
}

bool is_game_over()
{
	return read_data().game_over;
}

Creature::Type get_defeated_by ()
{
	return read_data().defeated_by;
}

void set_game_over(Creature::Type defeated_by)
{
	s_player_data.game_over = true;
	s_player_data.defeated_by = defeated_by;
}

int current_level ()
{
	return read_data().level;
}

int current_xp ()
{
	return read_data().xp;
}

int next_xp_threshold ()
{
	int const level = read_data().level;
	return 50 * Math::RoundToInt(pow(2, level - 1));
}

void gain_xp_for (Creature::Type creature_type)
{
	float const difficulty = Gingerbread::read(creature_type).difficulty;
	int const gain = Math::RoundToInt(10.0f * pow(2.0f, difficulty));
	edit_data().xp += gain;

	while (current_xp() >= next_xp_threshold())
	{
		edit_data().xp -= next_xp_threshold();
		++ edit_data().level;

		Gingerbread::edit_player_stats().max_hp += 2;
		Gingerbread::edit_player_stats().skill_magic += 5;
		handle().heal_hp(2);

		Draw::add_message("Welcome to level " + std::to_string(current_level()) + ".");
	}
}

} // namespace Player