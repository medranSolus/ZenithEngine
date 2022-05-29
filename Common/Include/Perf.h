#pragma once
#include "Types.h"
#include <string>
#include <map>

#ifdef _ZE_PLATFORM_WINDOWS
#include "Platform/WinAPI/Perf.h"
namespace ZE { typedef WinAPI::Perf PlatformPerf; }
#else
#error Missing Perf platform specific implementation!
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
		std::string lastTag = "";
		U64 stamp = 0;
		PlatformPerf platformImpl;

		void Save();

		Perf() = default;

	public:
		ZE_CLASS_DEFAULT(Perf);
		~Perf();

		static Perf& Get() noexcept { static Perf perf; return perf; }

		void Start(const std::string& sectionTag) noexcept;
		long double Stop() noexcept;
	};
}

#define ZE_PERF_START(sectionTag) ZE::Perf::Get().Start(sectionTag)
#define ZE_PERF_STOP() ZE::Perf::Get().Stop()