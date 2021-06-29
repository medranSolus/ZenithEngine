#pragma once
#include "GFX/DeferredContext.h"
#include "CommandList.h"
#include "Device.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11
{
	class DeferredContext : public GFX::DeferredContext
	{
		DX::ComPtr<ID3D11DeviceContext4> context = nullptr;
		ID3D11CommandList** commands = nullptr;

	public:
		DeferredContext(Device& dev);
		virtual ~DeferredContext() = default;

		ID3D11DeviceContext4* GetContext() const noexcept { return context.Get(); }
		void SetCommandList(GFX::CommandList& cl) noexcept override { commands = ((DX11::CommandList&)cl).GetList(); }

		void DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) override;
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) override;
		void FinishList(GFX::Device& dev) override;
	};
}