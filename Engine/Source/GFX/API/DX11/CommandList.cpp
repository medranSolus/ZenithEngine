#include "GFX/API/DX11/CommandList.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::GFX::API::DX11
{
	CommandList::CommandList(GFX::Device& dev)
	{
		ZE_DX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);
		deferred = false;

		ZE_DX_THROW_FAILED(tempCtx.As(&context));
#if _ZE_GFX_MARKERS
		ZE_DX_THROW_FAILED(context.As(&tagManager));
#endif
	}

	CommandList::CommandList(GFX::Device& dev, QueueType type)
	{
		ZE_DX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));
		deferred = true;

		ZE_DX_THROW_FAILED(tempCtx.As(&context));
#if _ZE_GFX_MARKERS
		ZE_DX_THROW_FAILED(context.As(&tagManager));
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
		if (deferred)
		{
			ZE_DX_ENABLE(dev.Get().dx11);
			ZE_DX_THROW_FAILED(context->FinishCommandList(FALSE, &commands));
		}
	}

	void CommandList::Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx11);
		ZE_DX_THROW_FAILED_INFO(context->Draw(vertexCount, 0));
	}

	void CommandList::DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx11);
		ZE_DX_THROW_FAILED_INFO(context->DrawIndexed(indexCount, 0, 0));
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx11);
		ZE_DX_THROW_FAILED_INFO(context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr));
		ZE_DX_THROW_FAILED_INFO(context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
		ZE_DX_THROW_FAILED_INFO(context->Draw(3, 0));
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API)
	{
		ZE_DX_ENABLE_INFO(dev.Get().dx11);
		ZE_DX_THROW_FAILED_INFO(context->Dispatch(groupX, groupY, groupZ));
	}

	void CommandList::Free(GFX::Device& dev) noexcept
	{
#if _ZE_GFX_MARKERS
		tagManager = nullptr;
#endif
		commands = nullptr;
		context = nullptr;
	}
}