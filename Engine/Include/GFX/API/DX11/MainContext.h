#pragma once
#include "GFX/MainContext.h"
#include "Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class MainContext : public GFX::MainContext
	{
		DX::ComPtr<ID3D11DeviceContext4> context = nullptr;

	public:
		MainContext(Device& dev);
		virtual ~MainContext() = default;

		ID3D11DeviceContext4* GetContext() const noexcept { return context.Get(); }

		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) override;
		// For best performance each thread group should be multiple of 32 (ideally as many as 2*processors on GPU)
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) override;
		void Execute(GFX::Device& dev, GFX::CommandList& cl) const override;
	};
}