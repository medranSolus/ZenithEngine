#pragma once
#include "Types.h"
#include <bitset>
#include <map>
#include <shared_mutex>
#include <string>

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
#if _ZE_MODE_PROFILE
		struct Data
		{
			long double AvgMicroseconds;
			U64 Count;
		};

		enum Flags : U8
		{
			SingleLineLogEntry,
			FlagsCount,
		};

		static constexpr const char* LOG_FILE = "perf_log_";
		static constexpr const char* LOG_FILE_EXT = ".txt";
		static constexpr bool MULTITHREADED = true;

		std::map<std::string, Data> data;
		std::vector<std::pair<U64, std::string>> lastTags;
		std::shared_mutex mutex;
		std::bitset<Flags::FlagsCount> flags = 0;
#endif
		PlatformPerf platformImpl;

#if _ZE_MODE_PROFILE
		U64& CreateStartStamp(const std::string& sectionTag) noexcept;
		long double SaveStopStamp(long double frequency, U64 stamp) noexcept;
#endif

		Perf() = default;

	public:
		ZE_CLASS_DEFAULT(Perf);
		~Perf();

		static Perf& Get() noexcept { static Perf perf; return perf; }

		double GetNow() noexcept;

#if _ZE_MODE_PROFILE
		constexpr void SetSingleLineLogEntry(bool val) noexcept { flags[Flags::SingleLineLogEntry] = val; }
		constexpr bool IsSingleLineLogEntry() const noexcept { return flags[Flags::SingleLineLogEntry]; }

		void Start(const std::string& sectionTag) noexcept;
		// Use for measuring short periods of time as it gets raw data based on RDTSC
		void StartShort(const std::string& sectionTag) noexcept;
		long double Stop() noexcept;
		// Use for measuring short periods of time as it gets raw data based on RDTSC
		long double StopShort() noexcept;
		void Save();
		U64 GetSectionCallCount(const std::string& sectionTag) noexcept;
#endif
	};
}

#if _ZE_MODE_PROFILE
// Use to configure tool behavior with given function
#	define ZE_PERF_CONFIGURE(function, val) ZE::Perf::Get().##function##(val)
#	define ZE_PERF_START(sectionTag) ZE::Perf::Get().Start(sectionTag)
// Use for measuring short periods of time as it gets raw data based on RDTSC
#	define ZE_PERF_START_SHORT(sectionTag) ZE::Perf::Get().StartShort(sectionTag)
#	define ZE_PERF_STOP() ZE::Perf::Get().Stop()
// Use for measuring short periods of time as it gets raw data based on RDTSC
#	define ZE_PERF_STOP_SHORT() ZE::Perf::Get().StopShort()
#	define ZE_PERF_COUNT(sectionTag) ZE::Perf::Get().GetSectionCallCount(sectionTag)
#else
// Use to configure tool behavior with given function
#	define ZE_PERF_CONFIGURE(function, val)
#	define ZE_PERF_START(sectionTag)
// Use for measuring short periods of time as it gets raw data based on RDTSC
#	define ZE_PERF_START_SHORT(sectionTag)
#	define ZE_PERF_STOP()
// Use for measuring short periods of time as it gets raw data based on RDTSC
#	define ZE_PERF_STOP_SHORT()
#	define ZE_PERF_COUNT(sectionTag)
#endif