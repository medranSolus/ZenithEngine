#pragma once
#include "Types.h"
#include <string>
#include <map>

#if _ZE_PLATFORM_WINDOWS
#include "Platform/WinAPI/Perf.h"
namespace ZE { typedef WinAPI::Perf PlatformPerf; }
#else
#	error Missing Perf platform specific implementation!
#endif

namespace ZE
{
	// CPU performance measurement tool
	class Perf final
	{
		struct Data
		{
			long double AvgMicroseconds;
			U64 Count;
		};

		static constexpr const char* LOG_FILE = "perf_log.txt";

		std::map<std::string, Data> data;
		std::vector<std::pair<U64, std::string>> lastTags;
		PlatformPerf platformImpl;

		void Save();
		U64& CreateStartStamp(const std::string& sectionTag) noexcept;
		long double SaveStopStamp(long double frequency, U64 stamp) noexcept;

		Perf() = default;

	public:
		ZE_CLASS_DEFAULT(Perf);
		~Perf();

		static Perf& Get() noexcept { static Perf perf; return perf; }

		void Start(const std::string& sectionTag) noexcept;
		// Use for measuring short periods of time as it gets raw data based on RDTSC
		void StartShort(const std::string& sectionTag) noexcept;
		long double Stop() noexcept;
		// Use for measuring short periods of time as it gets raw data based on RDTSC
		long double StopShort() noexcept;
		U64 GetSectionCallCount(const std::string& sectionTag) noexcept;
	};
}

#define ZE_PERF_START(sectionTag) ZE::Perf::Get().Start(sectionTag)
// Use for measuring short periods of time as it gets raw data based on RDTSC
#define ZE_PERF_START_SHORT(sectionTag) ZE::Perf::Get().StartShort(sectionTag)
#define ZE_PERF_STOP() ZE::Perf::Get().Stop()
// Use for measuring short periods of time as it gets raw data based on RDTSC
#define ZE_PERF_STOP_SHORT() ZE::Perf::Get().StopShort()
#define ZE_PERF_COUNT(sectionTag) ZE::Perf::Get().GetSectionCallCount(sectionTag)