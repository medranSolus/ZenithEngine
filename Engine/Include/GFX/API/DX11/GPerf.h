#pragma once
#include "GFX/CommandList.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class GPerf final
	{
		DX::ComPtr<ID3D11Query> disjoint;
		DX::ComPtr<ID3D11Query> begin;
		DX::ComPtr<ID3D11Query> end;

	public:
		GPerf(GFX::Device& dev);
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = delete;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = delete;
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX11"; }

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}