#include "GFX/API/DX12/Context.h"
//#include "GFX/API/DX12/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX12
{
	Context::Context(GFX::Device& dev, bool main)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		if (main)
			dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);
		else
		{
			ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));
		}
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context->QueryInterface(IID_PPV_ARGS(&tagManager)));
#endif
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