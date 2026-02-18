#include "BearLibTerminal.h"
#include "Player.h"

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Gingerbread.h"
#include "Math.h"
#include "Serialize.h"
#include "Spell.h"

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

	// Don't need to serialize these since we won't save in the middle of a turn.
	assert(automove == c_Invalid);
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
	return read_data().automove != c_CompassInvalid;
}

CompassDirection get_automove ()
{
	return read_data().automove;
}

void set_name (std::string str)
{
	edit_data().name = str;
}

void start_automove(CompassDirection dir)
{
	edit_data().automove = dir;
}

void stop_automove()
{
	edit_data().automove = c_CompassInvalid;
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

		// Hack!  Automatically learn spells when levelling up...
		// TODO: Implement a better way to learn spells.
		Spell::Index new_spell = (Spell::Index)read_data().level;
		if (Spell::is_valid_index(new_spell))
		{
			handle().learn_spell(new_spell);
		}

		Draw::add_message("Welcome to level " + std::to_string(current_level()) + ".");
	}
}

} // namespace Player