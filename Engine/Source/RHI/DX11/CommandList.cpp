#include "RHI/DX11/CommandList.h"
#include "GFX/Resource/PipelineStateCompute.h"
#include "GFX/Resource/PipelineStateGfx.h"

namespace ZE::RHI::DX11
{
	Expected<CommandList> CommandList::Create(GFX::Device& dev) noexcept
	{
		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);

		CommandList cl;
		cl.deferred = false;
		ZE_DX_RET_FAILED_EXPECT(tempCtx.As(&cl.context));
#if _ZE_GFX_MARKERS
		ZE_DX_RET_FAILED_EXPECT(cl.context.As(&cl.tagManager));
#endif
		return cl;
	}

	Expected<CommandList> CommandList::Create(GFX::Device& dev, GFX::QueueType type) noexcept
	{
		DX::ComPtr<ID3D11DeviceContext3> tempCtx = nullptr;
		ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));

		CommandList cl;
		cl.deferred = true;
		ZE_DX_RET_FAILED_EXPECT(tempCtx.As(&cl.context));
#if _ZE_GFX_MARKERS
		ZE_DX_RET_FAILED_EXPECT(cl.context.As(&cl.tagManager));
#endif
		return cl;
	}

	Status CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) const noexcept
	{
		pso.Get().dx11.Bind(context.Get());
		return {};
	}

	Status CommandList::Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) const noexcept
	{
		pso.Get().dx11.Bind(context.Get());
		return {};
	}

	Status CommandList::Close(GFX::Device& dev) noexcept
	{
		if (deferred)
		{
			ZE_DX_RET_FAILED(context->FinishCommandList(FALSE, &commands));
		}
		return {};
	}

	void CommandList::DrawFullscreen(GFX::Device& dev) const noexcept
	{
		ZE_DX_CHECK_FAILED(context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr), "Reset of vertex buffer for fullscreen draw generated debug messages!");
		ZE_DX_CHECK_FAILED(context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0), "Reset of index buffer for fullscreen draw generated debug messages!");
		ZE_DX_CHECK_FAILED(context->Draw(3, 0), "Fullscreen draw produced debug layer messages!");
	}

	void CommandList::Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept
	{
		ZE_DX_CHECK_FAILED(context->Dispatch(groupX, groupY, groupZ), "Dispatch produced debug layer messages!");
	}
}