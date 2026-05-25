#include "BearLibTerminal.h"
#include "Player.h"

#include "Action.h"
#include "Bot.h"
#include "Colour.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Math.h"
#include "Pathfind.h"
#include "Score.h"
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

	s.srz_float(sugar);

	s.srz_int(miscast_turn);
	s.srz_value(miscast_spell);

	// Don't need to serialize these since we won't save in the middle of a turn.
	assert(!acted);
	assert(ending == Score::Ending::Unfinished);
	assert(defeated_by.type == Damage::Cause::None);
	
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
	edit_data().automove_destination_creature.invalidate();
}

void start_pathfind (Vec3 target)
{
	if (World::read().get_visibility(target) == Visibility::Hidden)
	{
		// TODO: I'd like to allow this, but it can send astar into a very long loop.
		Draw::add_message("Can't auto-travel to unexplored square.");
		return;
	}

	if (World::read().get_visibility(target) != Visibility::Hidden &&
		World::read().is_solid(target))
	{
		Draw::add_message("Can't travel to that square.");
		return;
	}

	Creature::Handle target_creature = Creature::creature_at_pos(target);
	if (!target_creature.visible())
	{
		target_creature.invalidate();
	}

	bool success = Bot::try_player_pathfind(target, target_creature);
	if (success)
	{
		edit_data().automove_type = AutomoveType::Path;
		edit_data().automove_destination_creature = target_creature;
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
		edit_data().automove_destination_creature.invalidate();
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
		edit_data().automove_destination_creature.invalidate();
	}
	else
	{
		Draw::add_message("This area seems fully explored.");
	}
}

void auto_explore ()
{
	edit_data().automove_type = AutomoveType::Explore;
	edit_data().automove_destination_creature.invalidate();
}

void stop_automove()
{
	edit_data().automove_type = AutomoveType::None;
	edit_data().automove_dir = c_CompassInvalid;
	edit_data().automove_destination_creature.invalidate();
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

			Vec3 const pos_after_move = Action::pos_after_move(Player::handle(), relative_move);
			Creature::Handle const creature = Creature::creature_at_pos(pos_after_move);
			if (creature.valid() && creature != read_data().automove_destination_creature)
			{
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
			Vec3 const pos_after_move = Action::pos_after_move(Player::handle(), move);
			Creature::Handle const creature = Creature::creature_at_pos(pos_after_move);
			if (creature.valid() &&
				creature != read_data().automove_destination_creature)
			{
				Player::stop_automove();
				return;
			}

			bool moved = Action::player_try_move(move);
			if (!moved)
			{
				Player::stop_automove();
				return;
			}
		}
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

float get_sugar ()
{
	return read_data().sugar;
}

int get_sugar_bonus ()
{
	float const sugar = read_data().sugar;
	float const adjusted = (sugar - 50.0f) / 4.0f;
	return Math::RoundToInt(adjusted);
}

char const* get_sugar_colour ()
{
	int const sugar_int = Math::RoundToInt(read_data().sugar);
	return (sugar_int >= 90) ? cstr_Green :
		(sugar_int >= 80) ? cstr_LightGreen :
		(sugar_int >= 70) ? cstr_LighterGreen :
		(sugar_int >= 60) ? cstr_LightestGreen :
		(sugar_int >= 50) ? cstr_White :
		(sugar_int >= 40) ? cstr_LightestOrange :
		(sugar_int >= 30) ? cstr_LighterFlame:
		(sugar_int >= 20) ? cstr_LighterRed :
		(sugar_int >= 10) ? cstr_LightRed :
		cstr_Red;
}

void tick_sugar ()
{
	float& sugar = edit_data().sugar;

	float constexpr c_LossRate = 1.0f / 700.0f;
	float const loss = c_LossRate * sugar;

	sugar = std::max(0.0f, sugar - loss);
	Player::handle().update_derived_stats();
}

void gain_sugar (int amount)
{
	float const gain = (float)amount;
	float& sugar = edit_data().sugar;
	sugar = std::min(100.0f, sugar + gain);
}

Spell::Index get_recent_miscast ()
{
	if (read_data().miscast_turn < Game::get_turn_number() - 1)
	{
		return Spell::None;
	}
	else
	{
		return read_data().miscast_spell;
	}
}

void set_miscasted (Spell::Index spell_index)
{
	edit_data().miscast_turn = Game::get_turn_number();
	edit_data().miscast_spell = spell_index;
}

bool is_game_over()
{
	return read_data().ending != Score::Ending::Unfinished;
}

Score::Ending get_ending()
{
	return read_data().ending;
}

Damage::Cause get_defeated_by ()
{
	return read_data().defeated_by;
}

void set_defeated (Damage::Cause defeated_by)
{
	s_player_data.ending = Score::Ending::Defeated;
	s_player_data.defeated_by = defeated_by;
}

void set_won ()
{
	s_player_data.ending = Score::Ending::Won;
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

int total_xp_spent ()
{
	int const level = read_data().level;
	return (50 * Math::RoundToInt(pow(2, level - 1))) - 50;

	// For example, if you are level 2, you have spent:
	//   (50 * 2^1) - 50
	// = 100 - 50
	// = 50

	// Or if you are level 4, you have spent:
	//   (50 * 2^3) - 50
	// = (50 * 8) - 50
	// = 400 - 50
	// = 350
	// = 50 + 100 + 200
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