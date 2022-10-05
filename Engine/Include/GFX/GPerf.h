#pragma once
#include "API/DX11/GPerf.h"
#include "API/DX12/GPerf.h"
#include "API/VK/GPerf.h"
#include <unordered_map>

namespace ZE::GFX
{
	// Measuring GPU performance
	class GPerf final
	{
		static constexpr const char* LOG_FILE = "gpu_perf_log.txt";

		ZE_API_BACKEND(GPerf);
		// Average micro seconds must be computed each time Stop is called using:
		// pair.first = (time - pair.first) / ++pair.second
		std::unordered_map<std::string, std::pair<long double, U64>> data;
		std::string lastTag = "";

		void Save();

		GPerf(Device& dev) { ZE_API_BACKEND_VAR.Init(dev); }

	public:
		ZE_CLASS_MOVE(GPerf);
		~GPerf() { if (data.size()) Save(); }

		static constexpr void SwitchApi(API::ApiType nextApi, Device& dev) { Get(dev).ZE_API_BACKEND_VAR.Switch(nextApi, dev); }

		// Main Gfx API

		static GPerf& Get(Device& dev) { static GPerf perf(dev); return perf; }

		constexpr void Stop(CommandList& cl) const noexcept { ZE_API_BACKEND_CALL(Stop, cl); }

		void Start(CommandList& cl, const std::string& sectionTag) noexcept;
		void Collect(Device& dev) noexcept;
	};
}

#define ZE_GPERF_START(dev, cl, sectionTag) ZE::GFX::GPerf::Get(dev).Start(cl, sectionTag)
#define ZE_GPERF_STOP(dev, cl) ZE::GFX::GPerf::Get(dev).Stop(cl)
#define ZE_GPERF_COLLECT(dev) ZE::GFX::GPerf::Get(dev).Collect(dev)