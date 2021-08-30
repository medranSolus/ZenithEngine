#pragma once
#include "GFX/CommandList.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12
{
	class GPerf final
	{
	public:
		GPerf(GFX::Device& dev) {}
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = delete;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = delete;
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX12"; }

		void Start(GFX::CommandList& cl) noexcept {}
		void Stop(GFX::CommandList& cl) const noexcept {}
		long double GetData(GFX::Device& dev) noexcept { return 0.0L; }
	};
}