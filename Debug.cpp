#include "Debug.h"

#if _DEBUG
namespace Debug
{
	bool s_enabled[Category::Count];

	void init()
	{
		s_enabled[Action] = false;
		s_enabled[Bot] = false;
		s_enabled[Item] = false;
		s_enabled[Line] = false;
		s_enabled[Map] = false;
		s_enabled[Memory] = false;
		s_enabled[Pathfind] = false;
		s_enabled[Serialize] = false;
		s_enabled[Spell] = false;
	}

	bool enabled(Category category)
	{
		return s_enabled[category];
	}

	void set_enabled(Category category, bool new_value)
	{
		s_enabled[category] = new_value;
	}

	void set_all_enabled(bool new_value)
	{
		for (int i = 0; i < Debug::Category::Count; ++i)
		{
			s_enabled[i] = new_value;
		}
	}

	char const* category_name(Category category)
	{
		switch(category)
		{
			case Action: return "Action";
			case Bot: return "Bot";
			case Item: return "Item";
			case Line: return "Line";
			case Map: return "Map";
			case Memory: return "Memory";
			case Pathfind: return "Pathfind";
			case Serialize: return "Serialize";
			case Spell: return "Spell";
		}
		DebugBreak();
		return "Error";
	}
}
#endif // _DEBUG
