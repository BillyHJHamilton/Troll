#include "BearLibTerminal.h"
#include "Player.h"

#include "Creature.h"
#include "Draw.h"

namespace Player
{

Player::Data s_player_data;

Player::Data const & read_data()
{
	return s_player_data;
}

Player::Data & edit_data()
{
	return s_player_data;
}

void Player::clear()
{
	s_player_data = Player::Data();
}

Vec3 pos()
{
	return handle().pos();
}

Creature::Handle Player::handle()
{
	return 0;
}

bool is_automoving()
{
	return read_data().automove != c_CompassInvalid;
}

CompassDirection get_automove ()
{
	return read_data().automove;
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

} // namespace Player