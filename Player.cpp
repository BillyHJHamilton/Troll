#include "BearLibTerminal.h"
#include "Player.h"

#include "Creature.h"
#include "Draw.h"

namespace Player
{

Player::Data s_player_data;

void Player::clear()
{
	s_player_data = Player::Data();
}

Vec3 pos()
{
	return handle().pos();
}

Player::Data& data()
{
	return s_player_data;
}

Creature::Handle Player::handle()
{
	return 0;
}

void set_acted(bool acted)
{
	s_player_data.acted = acted;
}

void set_game_over(Creature::Type defeated_by)
{
	s_player_data.game_over = true;
	s_player_data.defeated_by = defeated_by;
}

}