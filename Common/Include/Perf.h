#pragma once
#include "Types.h"
#include <string>
#include <map>

namespace ZE
{
	class Perf
	{
		static constexpr const char* LOG_FILE = "perf_log.txt";

		std::map<std::string, std::pair<U64, U64>> data;
		std::string lastTag = "";
		U64 stamp = 0;

		void Save();

	public:
		Perf() = default;
		Perf(Perf&&) = default;
		Perf(const Perf&) = default;
		Perf& operator=(Perf&&) = default;
		Perf& operator=(const Perf&) = default;
		~Perf();

		void Start(const std::string& sectionTag) noexcept;
		void Stop() noexcept;
	};
}

#define ZE_PERF_SET() static Perf __perf
#define ZE_PERF_START(sectionTag) __perf.Start(sectionTag)
#define ZE_PERF_STOP() __perf.Stop()