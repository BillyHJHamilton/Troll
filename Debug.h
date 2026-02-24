#pragma once

#include <cassert>
#include <iostream>
#include <string>

namespace Debug
{
	enum Category
	{
		Action,
		Bot,
		Item,
		Line,
		Map,
		Memory,
		Serialize,
		Spell,
		Count
	};

	#ifdef _DEBUG
	void init();
	bool enabled(Category category);
	void set_enabled(Category category, bool new_value);
	void set_all_enabled(bool new_value);
	char const* category_name(Category category);
	#else
	inline void init() {}
	constexpr bool enabled(Category category) { return false; }
	inline void set_enabled(Category category, bool new_value) {}
	inline void set_all_enabled(bool new_value) {}
	inline char const* category_name(Category category) { return ""; }
	#endif
}

inline void DebugBreak(std::string msg = "Error")
{
#ifdef _DEBUG
	std::cerr << msg << std::endl;
	__debugbreak();
#endif
}

inline bool Check(bool value, std::string msg = "Error")
{
#ifdef _DEBUG
	if (!value)
	{
		std::cerr << msg << std::endl;
		__debugbreak();
	}
#endif
	return value;
}
