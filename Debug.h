#pragma once

#include <cassert>
#include <iostream>
#include <string>

#ifdef _DEBUG
static constexpr bool c_ShowActionDebug		= false;
static constexpr bool c_ShowBotDebug		= false;
static constexpr bool c_ShowLineDebug		= false;
static constexpr bool c_ShowMapDebug		= false;
static constexpr bool c_ShowPathfindDebug	= false;
static constexpr bool c_ShowSpellDebug		= true;
#else
static constexpr bool c_ShowActionDebug		= false;
static constexpr bool c_ShowBotDebug		= false;
static constexpr bool c_ShowLineDebug		= false;
static constexpr bool c_ShowMapDebug		= false;
static constexpr bool c_ShowPathfindDebug	= false;
static constexpr bool c_ShowSpellDebug		= false;
#endif

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
