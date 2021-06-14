#pragma once
#include "Graphics.h"

namespace ZE::GFX
{
	class GPerf final
	{
		static constexpr const char* LOG_FILE = "gpu_perf_log.txt";

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx;
		Microsoft::WRL::ComPtr<ID3D11Query> disjoint;
		Microsoft::WRL::ComPtr<ID3D11Query> begin;
		Microsoft::WRL::ComPtr<ID3D11Query> end;
		std::map<std::string, std::pair<long double, U64>> data;
		std::string lastTag = "";

		void Save();

		GPerf(Graphics& gfx);

	public:
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = default;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = default;
		~GPerf();

		static GPerf& Get(Graphics& gfx) { static GPerf perf(gfx); return perf; }

		void Start(const std::string& sectionTag) noexcept;
		void Stop() noexcept;
	};
}

#define ZE_GPERF_START(gfx, sectionTag) ZE::GFX::GPerf::Get(gfx).Start(sectionTag)
#define ZE_GPERF_STOP(gfx) ZE::GFX::GPerf::Get(gfx).Stop()