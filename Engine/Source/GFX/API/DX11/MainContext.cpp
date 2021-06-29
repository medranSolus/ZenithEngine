#include "GFX/API/DX11/MainContext.h"
#include "GFX/API/DX11/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	MainContext::MainContext(Device& dev)
	{
		ZE_GFX_ENABLE(dev);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		dev.GetDevice()->GetImmediateContext3(&tempCtx);
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&context)));
	}

	void MainContext::DrawIndexed(GFX::Device& dev, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(((Device&)dev));
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(count, 0, 0));
	}

	void MainContext::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(((Device&)dev));
		ZE_GFX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}

	void MainContext::Execute(GFX::Device& dev, GFX::CommandList& cl) const
	{
		assert(*((CommandList&)cl).GetList() != nullptr);
		ZE_GFX_ENABLE_INFO(((Device&)dev));
		ZE_GFX_THROW_FAILED_INFO(context->ExecuteCommandList(*((CommandList&)cl).GetList(), FALSE));
	}
}