#include "WorldLineItr.h"
#include "Stairs.h"
#include "World.h"

void WorldLineItr::advance()
{
	Vec3 const old = current();
	itr.advance();
	z += World::read().get_stairs_dz(old, *itr);
}

void WorldLineItr::advance_and_loop()
{
	Vec3 const old = current();
	itr.advance_and_loop();
	z += World::read().get_stairs_dz(old, *itr);
}
