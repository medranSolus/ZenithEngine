#include "GFX/Context.h"
#include "GFX/API/DX11/CommandList.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11
{
	Context::Context(GFX::Device& dev)
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		dev.Get().dx11.GetDevice()->GetImmediateContext3(&tempCtx);
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

	void Context::Execute(GFX::Device& dev, GFX::CommandList* cl, U32 count) const noexcept(ZE_NO_DEBUG)
	{
		assert(count && cl);
		for (U32 i = 0; i < count; ++i)
			Execute(dev, cl[i]);
	}

	void Context::Execute(GFX::Device& dev, GFX::Context& ctx) const
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11CommandList> cl;
		ZE_GFX_THROW_FAILED(ctx.Get().dx11.GetContext()->FinishCommandList(FALSE, cl.GetAddressOf()));
		ZE_GFX_THROW_FAILED_INFO(ctx.Get().dx11.GetContext()->ExecuteCommandList(cl.Get(), FALSE));
	}

	void Context::Execute(GFX::Device& dev, GFX::Context* ctx, U32 count) const
	{
		assert(count && ctx);
		for (U32 i = 0; i < count; ++i)
			Execute(dev, ctx[i]);
	}

	void Context::CreateList(GFX::Device& dev, GFX::CommandList& cl) const
	{
		assert(cl.Get().dx11.GetList() != nullptr);
		ZE_GFX_ENABLE(dev.Get().dx11);
		ZE_GFX_THROW_FAILED(context->FinishCommandList(FALSE, cl.Get().dx11.GetList()));
	}

	void Context::CreateDeffered(GFX::Device& dev, GFX::Context& ctx) const
	{
		ZE_GFX_ENABLE(dev.Get().dx11);

		DX::ComPtr<ID3D11DeviceContext3> tempCtx;
		ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateDeferredContext3(0, &tempCtx));

		new(&ctx.Get().dx11) Context;
		ZE_GFX_THROW_FAILED(tempCtx->QueryInterface(IID_PPV_ARGS(&ctx.Get().dx11.context)));
#ifdef _ZE_MODE_DEBUG
		ZE_GFX_THROW_FAILED(ctx.Get().dx11.context->QueryInterface(IID_PPV_ARGS(&ctx.Get().dx11.tagManager)));
#endif
	}
}