#pragma once
#if _ZE_RHI_DX11
#	include "RHI/DX11/GPerf.h"
#endif
#if _ZE_RHI_DX12
#	include "RHI/DX12/GPerf.h"
#endif
#if _ZE_RHI_VK
#	include "RHI/VK/GPerf.h"
#endif
#include <unordered_map>

namespace ZE::GFX
{
	// Measuring GPU performance
	class GPerf final
	{
		static constexpr const char* LOG_FILE = "gpu_perf_log.txt";

		std::bitset<2> singletonStatus = 0;
		ZE_RHI_BACKEND(GPerf);
		// Average micro seconds must be computed each time Stop is called using:
		// pair.first = (time - pair.first) / ++pair.second
		std::unordered_map<std::string, std::pair<long double, U64>> data;
		std::string lastTag = "";

		static Expected<GPerf> Create(Device& dev) noexcept { ZE_RHI_BACKEND_CREATE(GPerf, dev); }

		constexpr bool IsInitialized() const noexcept { return singletonStatus[0]; }
		constexpr bool IsCreated() const noexcept { return singletonStatus[1]; }

		void Save() noexcept;

		GPerf() = default;

	public:
		ZE_CLASS_MOVE(GPerf);
		~GPerf() { if (data.size()) Save(); }

		// Main Gfx API

		static GPerf& Get(Device& dev) noexcept;

		constexpr void Stop(CommandList& cl) const noexcept { if (IsInitialized()) { ZE_RHI_BACKEND_CALL(Stop, cl); } }

		void Start(CommandList& cl, const std::string& sectionTag) noexcept;
		void Collect(Device& dev) noexcept;
	};
}

#define ZE_GPERF_START(dev, cl, sectionTag) ZE::GFX::GPerf::Get(dev).Start(cl, sectionTag)
#define ZE_GPERF_STOP(dev, cl) ZE::GFX::GPerf::Get(dev).Stop(cl)
#define ZE_GPERF_COLLECT(dev) ZE::GFX::GPerf::Get(dev).Collect(dev)