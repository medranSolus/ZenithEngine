#include "Perf.h"
#include "Intrinsics.h"

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
		lastTag = sectionTag;
		stamp = platformImpl.GetCurrentTimestamp();
	}

	long double Perf::Stop() noexcept
	{
		// Get timestamp and convert to microseconds elapsed time
		stamp = (platformImpl.GetCurrentTimestamp() - stamp) * 1000000ULL;
		const long double time = static_cast<long double>(stamp) / platformImpl.GetFrequency();

		// Combine with rest of data
		Data& dataPoint = data.at(lastTag);
		dataPoint.AvgMicroseconds += (time - dataPoint.AvgMicroseconds) / static_cast<long double>(++dataPoint.Count);

		stamp = 0;
		lastTag = "";
		return time;
	}
}