#include "GFX/API/DX11/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::API::DX11
{
	CommandList::CommandList(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);

		ZE_GFX_THROW_FAILED(tempCtx.As(&context));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context.As(&tagManager));
#endif
	}

	CommandList::CommandList(GFX::Device& dev, CommandType type)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));

		ZE_GFX_THROW_FAILED(tempCtx.As(&context));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(context.As(&tagManager));
#endif
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso)
	{
		pso.Get().dx11.Bind(context.Get());
	}

	void CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso)
	{
		pso.Get().dx11.Bind(context.Get());
	}

	void CommandList::Close(GFX::Device& dev)
	{
		if (context->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED)
		{
			ZE_GFX_ENABLE(dev.Get().dx11);
			ZE_GFX_THROW_FAILED(context->FinishCommandList(FALSE, &commands));
		}
	}

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->Draw(vertexCount, 0));
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->DrawIndexed(indexCount, 0, 0));
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));
		ZE_GFX_THROW_FAILED_INFO(context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
		ZE_GFX_THROW_FAILED_INFO(context->Draw(3, 0));
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG)
	{
		ZE_GFX_ENABLE_INFO(dev.Get().dx11);
		ZE_GFX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}
}