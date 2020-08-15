#include "BearLibTerminal.h"
#include "Player.h"

#include "Creature.h"
#include "Draw.h"
#include "Global.h"
#include "Map.h"

Player global_player;
Player & g_player () { return global_player; }

Vec2 const & Player::pos ()
{
	return handle().pos();
}

bool Player::try_move (Vec2 const & relative_move)
{
	Vec2 new_pos = Player::pos() + relative_move;

	Map const & map = g_map();

	if (map.tile_is_solid(new_pos))
	{
		return false;
	}
	else if (Creature::creature_at_pos(new_pos) != Creature::None)
	{
		return false;
	}
	else
	{
		handle().move(new_pos);
		return true;
	}
}
