#pragma once

#include "Types.h"

namespace Damage
{
	enum Type : int
	{
		None = -1,
		Basic = 0,
		ToLife,  // doesn't affect Features
		Fire,
		Acid,
		Count,
	};

	struct Cause
	{
		Cause() = default;
		Cause(Creature::Handle const& creature);
		Cause(Creature::Type creature_type);
		Cause(Status::Index status_index);
		Cause(Cloud::Type cloud_type);

		enum Type : int
		{
			None = -1,
			Creature,	// index is Creature::Type
			Status,		// index is Status::Index
			Cloud,		// index is Cloud::Type
			//Misc,
		};
		Cause::Type type = Cause::None;
		int index = c_Invalid;
	};

	struct Packet
	{
		int amount = 0;
		Type type = None;
		Cause cause = {};
	};
}
