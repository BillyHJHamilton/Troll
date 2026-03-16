#include "Damage.h"
#include "Creature.h"

namespace Damage
{

Cause::Cause(Creature::Handle const& creature) :
	type(Type::Creature),
	index((int)creature.type())
{ }

Cause::Cause(Creature::Type creature_type) :
	type(Type::Creature),
	index((int)creature_type)
{ }

Cause::Cause(Status::Index status_index) :
	type(Type::Status),
	index((int)status_index)
{ }

Cause::Cause(Cloud::Type cloud_type) :
	type(Type::Cloud),
	index((int)cloud_type)
{ }

} // namespace Damage
