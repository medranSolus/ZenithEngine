#include "GFX/Pipeline/RenderPass/Base/RenderPass.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	void RenderPass::BindResources(Graphics& gfx)
	{
		if (renderTarget == nullptr)
		{
			depthStencil->Unbind(gfx);
			depthStencil->DepthStencil::Bind(gfx);
		}
		else
		{
			renderTarget->Unbind(gfx);
			if (depthStencil == nullptr)
				renderTarget->BindTarget(gfx);
			else
			{
				depthStencil->Unbind(gfx);
				renderTarget->BindTarget(gfx, *depthStencil);
			}
		}
	}

	void RenderPass::Finalize()
	{
		if (renderTarget == nullptr && depthStencil == nullptr)
			throw ZE_RGC_EXCEPT("RenderPass \"" + GetName() + "\" needs at least one RenderTarget or DepthStencil!");
		BindingPass::Finalize();
	}
}