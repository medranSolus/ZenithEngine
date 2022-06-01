#include "Perf.h"
#include <fstream>

namespace ZE
{
	void Perf::Save()
	{
		std::ofstream fout(LOG_FILE, std::ios_base::app);
		if (!fout.good())
			return;
		for (auto& x : data)
		{
			fout << '[' << x.first << "] Avg time: " << x.second.AvgMicroseconds << " us, tests: " << x.second.Count << std::endl;
		}
		data.clear();
		fout.close();
	}

	Perf::Perf() noexcept
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		frequency = static_cast<long double>(freq.QuadPart);
	}

	Perf::~Perf()
	{
		if (data.size() > 0)
			Save();
	}

	void Perf::Start(const std::string& sectionTag) noexcept
	{
		std::string tag = "";
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag + tag, Data(0.0L, 0ULL));
		lastTags.emplace_back(0ULL, sectionTag).first = GetCurrentTimestamp();
	}

	long double Perf::Stop() noexcept
	{
		U64 stamp = GetCurrentTimestamp();
		assert(lastTags.size() && "Incorrect Start-Stop calling of performance measurements!");

		// Get timestamp and convert to microseconds elapsed time
		stamp = (stamp - lastTags.back().first) * 1000000ULL;
		const long double time = static_cast<long double>(stamp) / frequency;

		// Combine with rest of data
		Data& dataPoint = data.at(lastTags.back().second);
		dataPoint.AvgMicroseconds += (time - dataPoint.AvgMicroseconds) / static_cast<long double>(++dataPoint.Count);

		lastTags.pop_back();
		return time;
	}

	U64 Perf::GetSectionCallCount(const std::string& sectionTag) noexcept
	{
		auto it = data.find(sectionTag);
		if (it == data.end())
			return 0;
		return it->second.Count;
	}
}