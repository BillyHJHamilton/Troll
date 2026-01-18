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
