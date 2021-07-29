#pragma once
#include "API/DX11/GPerf.h"
#include "Device.h"
#include <unordered_map>

namespace ZE::GFX
{
	// Measuring GPU performance
	class GPerf final
	{
		static constexpr const char* LOG_FILE = "gpu_perf_log.txt";

	protected:
		static inline GPerf* instance = nullptr;

		ZE_API_BACKEND(GPerf) backend;
		// Average micro seconds must be computed each time Stop is called using:
		// pair.first = (time - pair.first) / ++pair.second
		std::unordered_map<std::string, std::pair<long double, U64>> data;
		std::string lastTag = "";

		void Save();

		GPerf(Device& dev) { backend.Init(dev); }

	public:
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = delete;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = delete;
		~GPerf() { if (data.size()) Save(); }

		static constexpr void Init(Device& dev) { assert(!instance); instance = new GPerf(dev); }
		static constexpr void SwitchApi(API::ApiType nextApi, Device& dev) { Get().backend.Switch(nextApi, dev); }
		static constexpr GPerf& Get() { assert(instance); return *instance; }

		void Start(const std::string& sectionTag) noexcept;
		void Stop() noexcept;
	};
}

#define ZE_GPERF_START(sectionTag) ZE::GFX::GPerf::Get().Start(sectionTag)
#define ZE_GPERF_STOP() ZE::GFX::GPerf::Get().Stop()