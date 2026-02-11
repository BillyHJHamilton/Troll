#pragma once

namespace House
{
	enum Id : int
	{
		None = -1,
		Gryffindor = 0,
		Hufflepuff = 1,
		Ravenclaw = 2,
		Slytherin = 3,
		Count = 4
	};

	inline bool is_valid(Id id) { return id > None && id < Count; }
	char const * name(Id id);
	char const * colour(Id id);
	char const * description(Id id);
}
