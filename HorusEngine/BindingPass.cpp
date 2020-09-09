#include "BindingPass.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void BindingPass::BindResources(Graphics& gfx)
	{
		Resource::IBufferResource::UnbindAll(gfx);
		if (renderTarget == nullptr)
		{
			depthStencil->Unbind(gfx);
			depthStencil->DepthStencil::Bind(gfx);
		}
		else
		{
			renderTarget->Unbind(gfx);
			if (depthStencil == nullptr)
				renderTarget->RenderTarget::Bind(gfx);
			else
			{
				depthStencil->Unbind(gfx);
				renderTarget->Bind(gfx, *depthStencil);
			}
		}
	}

	void BindingPass::BindAll(Graphics& gfx)
	{
		BindResources(gfx);
		for (auto& bind : binds)
			bind->Bind(gfx);
	}

	void BindingPass::Finalize() const
	{
		if (renderTarget == nullptr && depthStencil == nullptr)
			throw RGC_EXCEPT("BindingPass \"" + GetName() + "\" needs at least one RenderTarget or DepthStencil!");
		BasePass::Finalize();
	}
}