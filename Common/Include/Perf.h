#pragma once
#include "Types.h"
#include <string>
#include <map>

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
		long double frequency;

		static U64 GetCurrentTimestamp() noexcept { LARGE_INTEGER stamp; QueryPerformanceCounter(&stamp); return stamp.QuadPart; }

		void Save();

		Perf() noexcept;

	public:
		Perf(Perf&&) = default;
		Perf(const Perf&) = default;
		Perf& operator=(Perf&&) = default;
		Perf& operator=(const Perf&) = default;
		~Perf();

		static Perf& Get() noexcept { static Perf perf; return perf; }

		void Start(const std::string& sectionTag) noexcept;
		long double Stop() noexcept;
		U64 GetSectionCallCount(const std::string& sectionTag) noexcept;
	};
}

#define ZE_PERF_START(sectionTag) ZE::Perf::Get().Start(sectionTag)
#define ZE_PERF_STOP() ZE::Perf::Get().Stop()
#define ZE_PERF_COUNT(sectionTag) ZE::Perf::Get().GetSectionCallCount(sectionTag)