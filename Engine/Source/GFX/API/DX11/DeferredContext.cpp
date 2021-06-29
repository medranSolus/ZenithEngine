#include "GFX/API/DX11/DeferredContext.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	DeferredContext::DeferredContext(Device& dev)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateDeferredContext3(0, &tempCtx));
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
	}

	void DeferredContext::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(((Device&)dev));
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0, 0));
	}

	void DeferredContext::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(((Device&)dev));
		ZE_GFX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}

	void DeferredContext::FinishList(GFX::Device& dev)
	{
		assert(commands);
		ZE_GFX_ENABLE(((Device&)dev));
		ZE_GFX_THROW_FAILED(context->FinishCommandList(FALSE, commands));
		commands = nullptr;
	}
}