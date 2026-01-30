#pragma once

#include <iostream>
#include <string>

inline void DebugBreak(std::string msg = "Error")
{
#ifdef _DEBUG
	std::cerr << msg << std::endl;
	__debugbreak();
#endif
}