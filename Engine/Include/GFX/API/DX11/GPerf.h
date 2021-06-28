#pragma once
#include "GFX/GPerf.h"
#include "D3D11.h"
#include "Device.h"

namespace ZE::GFX::API::DX11
{
	class GPerf : public GFX::GPerf
	{
		DX::ComPtr<ID3D11DeviceContext> ctx;
		DX::ComPtr<ID3D11Query> disjoint;
		DX::ComPtr<ID3D11Query> begin;
		DX::ComPtr<ID3D11Query> end;

	protected:
		constexpr const char* GetApiString() const noexcept override { return "DX11"; }

		void StartImpl() noexcept override;
		void StopImpl() noexcept override;

	public:
		GPerf(Device& dev);
		GPerf(GPerf&&) = default;
		GPerf(const GPerf&) = default;
		GPerf& operator=(GPerf&&) = default;
		GPerf& operator=(const GPerf&) = default;
		virtual ~GPerf() = default;
	};
}