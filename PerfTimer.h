#pragma once

#ifdef RELEASE
	#define d_PerfTest false
#else
	#define d_PerfTest true
#endif

#if d_PerfTest
#include <chrono>
#endif

// Limitations:
//  - Multiple timers with the same name (in different places) are not supported.
class PerfTimer
{
public:
	PerfTimer(const char* timerName);
	~PerfTimer();

	static void PrintAll();

private:
#if d_PerfTest
	const char* m_TimerName;
	std::chrono::steady_clock::time_point m_StartTime;
#endif
};
