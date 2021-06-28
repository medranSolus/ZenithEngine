#pragma once
#include "Device.h"
#include <unordered_map>

namespace ZE::GFX
{
	// Measuring GPU performance
	class GPerf
	{
		static constexpr const char* LOG_FILE = "gpu_perf_log.txt";

	protected:
		static inline GPerf* instance = nullptr;

		// Average micro seconds must be computed each time Stop is called using:
		// pair.first = (time - pair.first) / ++pair.second
		std::unordered_map<std::string, std::pair<long double, U64>> data;
		std::string lastTag = "";
		
		virtual const char* GetApiString() const noexcept = 0;
		virtual void StartImpl() noexcept = 0;
		virtual void StopImpl() noexcept = 0;
		void Save();

		GPerf() = default;

	public:
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = default;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = default;
		virtual ~GPerf();

		static GPerf& Get() { assert(instance); return *instance; }

		static void Init(Device& dev);

		void Start(const std::string& sectionTag) noexcept;
		void Stop() noexcept;
	};
}

#define ZE_GPERF_START(sectionTag) ZE::GFX::GPerf::Get().Start(sectionTag)
#define ZE_GPERF_STOP() ZE::GFX::GPerf::Get().Stop()