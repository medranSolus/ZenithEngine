#pragma once
#include "Types.h"
#include <string>
#include <map>

namespace ZE
{
	class Perf final
	{
		static constexpr const char* LOG_FILE = "perf_log.txt";

		std::map<std::string, std::pair<U64, U64>> data;
		std::string lastTag = "";
		U64 stamp = 0;

		void Save();

		Perf() = default;

	public:
		Perf(Perf&&) = default;
		Perf(const Perf&) = default;
		Perf& operator=(Perf&&) = default;
		Perf& operator=(const Perf&) = default;
		~Perf();

		static Perf& Get() noexcept { static Perf perf; return perf; }

		void Start(const std::string& sectionTag) noexcept;
		void Stop() noexcept;
	};
}

#define ZE_PERF_START(sectionTag) ZE::Perf::Get().Start(sectionTag)
#define ZE_PERF_STOP() ZE::Perf::Get().Stop()