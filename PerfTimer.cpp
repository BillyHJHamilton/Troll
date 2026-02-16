#include "PerfTimer.h"

#if d_PerfTest
#include <iomanip>
#include <iostream>
#include "MapUtil.h"
#include "VectorUtil.h"

namespace PerfTimerPrivate
{
	struct Info
	{
		double m_TotalTime = 0.0;
		int m_NumHits = 0;
	};

	static std::unordered_map<const char*,Info> s_Map;
}

PerfTimer::PerfTimer(const char* timerName) :
	m_TimerName(timerName),
	m_StartTime(std::chrono::steady_clock::now())
{
}

PerfTimer::~PerfTimer()
{
	auto const endTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration duration = (endTime - m_StartTime);
	double const elapsedSeconds = std::chrono::duration<double>(duration).count();
	//double endTime = GameApp::Get().GetClockTime();
	PerfTimerPrivate::Info& info = Util::FindOrAdd(PerfTimerPrivate::s_Map, m_TimerName);
	info.m_TotalTime += (elapsedSeconds);
	++info.m_NumHits;
}

void PerfTimer::PrintAll()
{
	struct TimerResult
	{
		const char* name;
		double value;
		int hits;

		bool operator>(const TimerResult& other) const
		{
			return value > other.value;
		}

		void print_value() const
		{
			if (value < 0.001)
			{
				std::cout << " " << std::setw(40) << name
					<< std::setw(10) << value*1'000'000.0 << " micro";
			}
			else if (value < 1.0)
			{
				std::cout << " " << std::setw(40) << name
					<< std::setw(10) << value*1'000.0 << " ms   ";
			}
			else if (value < 1000.0)
			{
				std::cout << " " << std::setw(40) << name
					<< std::setw(10) << value << " s    ";
			}
			else
			{
				std::cout << " " << std::setw(40) << name
					<< std::setw(10) << value/1'000 << " ks   ";
			}
		}
	};

	// Sort the timers from longest to smallest.
	std::vector<TimerResult> list;
	for (const auto& pair : PerfTimerPrivate::s_Map)
	{
		const char* name = pair.first;
		const PerfTimerPrivate::Info& info = pair.second;
		const double avg = info.m_TotalTime / static_cast<double>(info.m_NumHits);
		list.push_back({name, avg, info.m_NumHits});
	}
	Util::SortDescending(list);

	// Print them.
	if (list.size() > 0)
	{
		std::cout << "Average Performance Results: " << std::endl
			<< std::fixed << std::setprecision(3);
		for (const TimerResult& timerResult : list)
		{
			timerResult.print_value();
			std::cout << "\n";

			//const double perSecond = 1.0 / avg;
			//std::cout << " (" << perSecond << "/second)" << std::endl;

			//const double pctBudget = 100.0 * (pair.value/(1.0/60.0));
			//std::cout << "   (" << pctBudget << "%)" << std::endl;
		}
		std::cout << std::endl;
	}

	// Also do the totals (non-averaged) and number of hits.

	std::vector<TimerResult> list_totals;
	for (const auto& pair : PerfTimerPrivate::s_Map)
	{
		const char* name = pair.first;
		const PerfTimerPrivate::Info& info = pair.second;
		const double total = info.m_TotalTime;
		list_totals.push_back({name, total, info.m_NumHits});
	}
	Util::SortDescending(list_totals);

	if (list_totals.size() > 0)
	{
		std::cout << "Total Costs (Sum of All Calls): " << std::endl
			<< std::fixed << std::setprecision(3);
		for (const TimerResult& timerResult : list_totals)
		{
			timerResult.print_value();
			std::cout << " (";
			std::cout << timerResult.hits;
			std::cout << + " hits)\n";
		}
		std::cout << std::endl;
	}
}

#else
PerfTimer::PerfTimer(const char* timerName)
{
}

PerfTimer::~PerfTimer()
{
}

void PerfTimer::PrintAll()
{
}
#endif