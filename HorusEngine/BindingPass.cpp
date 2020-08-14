#include "BindingPass.h"
#include "RenderGraphCompileException.h"

namespace GFX::Pipeline::RenderPass::Base
{
	void BindingPass::BindResources(Graphics& gfx) noexcept
	{
		if (renderTarget == nullptr)
			depthStencil->Bind(gfx);
		else
		{
			Resource::RenderTarget::UnbindAll(gfx);
			renderTarget->Unbind(gfx);
			if (depthStencil == nullptr)
				renderTarget->RenderTarget::Bind(gfx);
			else
				renderTarget->Bind(gfx, *depthStencil);
		}
	}

	void BindingPass::BindAll(Graphics& gfx) noexcept
	{
		BindResources(gfx);
		for (auto& bind : binds)
			bind->Bind(gfx);
	}

	void BindingPass::Finalize()
	{
		if (renderTarget == nullptr && depthStencil == nullptr)
			throw RGC_EXCEPT("BindingPass \"" + GetName() + "\" needs at least one RenderTarget or DepthStencil!");
		BasePass::Finalize();
	}
}