#pragma once
#include "GFX/Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class GPerf final
	{
		DX::ComPtr<ID3D11DeviceContext> ctx;
		DX::ComPtr<ID3D11Query> disjoint;
		DX::ComPtr<ID3D11Query> begin;
		DX::ComPtr<ID3D11Query> end;

	public:
		GPerf(GFX::Device& dev);
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = default;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = default;
		~GPerf() = default;

		static constexpr const char* GetApiString() noexcept { return "DX11"; }

		void Start() noexcept;
		long double Stop() noexcept;
	};
}