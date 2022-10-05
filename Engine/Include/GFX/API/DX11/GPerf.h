#pragma once
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11
{
	class GPerf final
	{
		DX::ComPtr<ID3D11Query> disjoint;
		DX::ComPtr<ID3D11Query> begin;
		DX::ComPtr<ID3D11Query> end;

	public:
		GPerf() = default;
		GPerf(GFX::Device& dev);
		ZE_CLASS_MOVE(GPerf);
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX11"; }

		void Start(GFX::CommandList& cl) noexcept;
		void Stop(GFX::CommandList& cl) const noexcept;
		long double GetData(GFX::Device& dev) noexcept;
	};
}