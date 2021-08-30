#pragma once
#include "API/DX11/GPerf.h"
#include "API/DX12/GPerf.h"
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
		std::vector<std::string> lastTags;

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

		constexpr void Stop(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(backend, Stop, cl); }

		void Start(CommandList& cl, const std::string& sectionTag) noexcept;
		void Collect(Device& dev) noexcept;
	};
}

#define ZE_GPERF_START(cl, sectionTag) ZE::GFX::GPerf::Get().Start(cl, sectionTag)
#define ZE_GPERF_STOP(cl) ZE::GFX::GPerf::Get().Stop(cl)
#define ZE_GPERF_COLLECT(dev) ZE::GFX::GPerf::Get().Collect(dev)