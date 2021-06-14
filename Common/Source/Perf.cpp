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
			if (x.second.second != 0)
				x.second.first /= x.second.second;
			fout << '[' << x.first << "] Avg cycles: " << x.second.first << ", tests: " << x.second.second << std::endl;
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
			data.emplace(sectionTag + tag, std::make_pair(0ULL, 0ULL));
		lastTag = sectionTag;
		__faststorefence();
		stamp = __rdtsc();
	}

	void Perf::Stop() noexcept
	{
		const U64 end = __rdtsc();
		__faststorefence();
		data.at(lastTag).first += end - stamp;
		++data.at(lastTag).second;
		stamp = 0;
		lastTag = "";
	}
}