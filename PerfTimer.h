#pragma once

#define PERF_TEST true

#if PERF_TEST
#include <chrono>
#endif

// Limitations:
//  - Multiple timers with the same name (in different places) are not supported.
//  - Recursive timers are not supported.
class PerfTimer
{
public:
	PerfTimer(const char* timerName);
	~PerfTimer();

	static void PrintAll();

private:
#if PERF_TEST
	const char* m_TimerName;
	std::chrono::steady_clock::time_point m_StartTime;
#endif
};
