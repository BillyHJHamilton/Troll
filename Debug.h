#pragma once

#include <cassert>
#include <iostream>
#include <string>

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
