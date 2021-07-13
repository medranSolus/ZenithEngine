#include "GFX/API/DX11/Context.h"
#include "GFX/API/DX11/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	Context::Context(Device& dev, bool main)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		if (main)
			dev.GetDevice()->GetImmediateContext3(&tempCtx);
		else
		{
			ZE_GFX_THROW_FAILED(dev.GetDevice()->CreateDeferredContext3(0, &tempCtx));
		}
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
	}

	void Context::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0, 0));
	}

	void Context::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}

	void Context::Execute(GFX::Device& dev, GFX::CommandList& cl) const noexcept(ZE_NO_DEBUG)
	{
		assert(*cl.Get().dx11.GetList() != nullptr);
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->ExecuteCommandList(*cl.Get().dx11.GetList(), FALSE));
	}

	void Context::CreateList(GFX::Device& dev, GFX::CommandList& cl) const
	{
		assert(cl.Get().dx11.GetList() != nullptr);
		ZE_GFX_ENABLE(dev.Get().dx11);
		ZE_GFX_THROW_FAILED(context->FinishCommandList(FALSE, cl.Get().dx11.GetList()));
	}
}