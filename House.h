#pragma once

namespace House
{
	enum Type : int
	{
		None = -1,
		Gryffindor = 0,
		Hufflepuff = 1,
		Ravenclaw = 2,
		Slytherin = 3,
		Count = 4
	};

	inline bool is_valid(Type house) { return house > None && house < Count; }
	char const * name(Type house);
	char const * colour(Type house);
	char const * description(Type house);
}
