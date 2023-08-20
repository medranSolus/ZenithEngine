#include "Perf.h"
#include "Intrinsics.h"

namespace ZE
{
	void Perf::Save()
	{
		std::ofstream fout(LOG_FILE, std::ios_base::app);
		if (!fout.good())
		{
			ZE_FAIL("Cannot open perf output file!");
			return;
		}
		if (IsSingleLineLogEntry())
		{
			for (auto& x : data)
				fout << '[' << x.first << "] Avg time: " << x.second.AvgMicroseconds << " us, tests: " << x.second.Count << std::endl;
		}
		else
		{
			for (auto& x : data)
			{
				fout << '[' << x.first << ']' << std::endl
					<< "    Avg time: " << x.second.AvgMicroseconds << " us" << std::endl
					<< "    Tests:    " << x.second.Count << std::endl;
			}
		}
		fout.close();
		data.clear();
	}

	U64& Perf::CreateStartStamp(const std::string& sectionTag) noexcept
	{
		if (data.find(sectionTag) == data.end())
			data.emplace(sectionTag, Data(0.0L, 0ULL));
		return lastTags.emplace_back(0ULL, sectionTag).first;
	}

	long double Perf::SaveStopStamp(long double frequency, U64 stamp) noexcept
	{
		ZE_ASSERT(lastTags.size(), "Incorrect Start-Stop calling of performance measurements!");

		// Get timestamp and convert to microseconds elapsed time
		stamp = (stamp - lastTags.back().first) * 1000000ULL;
		const long double time = static_cast<long double>(stamp) / frequency;

		// Combine with rest of data
		Data& dataPoint = data.at(lastTags.back().second);
		dataPoint.AvgMicroseconds += (time - dataPoint.AvgMicroseconds) / static_cast<long double>(++dataPoint.Count);

		lastTags.pop_back();
		return time;
	}

	Perf::~Perf()
	{
		if (data.size() > 0)
			Save();
	}

	void Perf::Start(const std::string& sectionTag) noexcept
	{
		LockGuardRW lock(mutex);
		CreateStartStamp(sectionTag) = platformImpl.GetCurrentTimestamp();
	}

	void Perf::StartShort(const std::string& sectionTag) noexcept
	{
		LockGuardRW lock(mutex);
		U64& stamp = CreateStartStamp(sectionTag);
		Intrin::FenceMemory();
		Intrin::FenceLoad();
		stamp = Intrin::Rdtsc();
	}

	long double Perf::Stop() noexcept
	{
		const U64 stamp = platformImpl.GetCurrentTimestamp();

		LockGuardRW lock(mutex);
		return SaveStopStamp(platformImpl.GetFrequency(), stamp);
	}

	long double Perf::StopShort() noexcept
	{
		U64 stamp = Intrin::Rdtsc();
		Intrin::FenceLoad();

		LockGuardRW lock(mutex);
		return SaveStopStamp(1.0L, stamp);
	}

	U64 Perf::GetSectionCallCount(const std::string& sectionTag) noexcept
	{
		LockGuardRW lock(mutex);
		auto it = data.find(sectionTag);
		if (it == data.end())
			return 0;
		return it->second.Count;
	}
}