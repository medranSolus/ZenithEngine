#include "GFX/API/DX11/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	CommandList::CommandList(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);

		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context->QueryInterface(IID_PPV_ARGS(&tagManager)));
#endif
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));

		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context->QueryInterface(IID_PPV_ARGS(&tagManager)));
#endif
	}

	void CommandList::Close(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);
		ZE_GFX_THROW_FAILED(context->FinishCommandList(FALSE, &commands));
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0, 0));
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}
}