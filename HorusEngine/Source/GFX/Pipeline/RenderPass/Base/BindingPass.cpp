#include "GFX/Pipeline/RenderPass/Base/BindingPass.h"

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
				renderTarget->BindTarget(gfx);
			else
			{
				depthStencil->Unbind(gfx);
				renderTarget->BindTarget(gfx, *depthStencil);
			}
		}
	}

	void BindingPass::AddBind(GfxResPtr<GFX::Resource::IBindable>&& bind) noexcept
	{
		binds.emplace_back(std::forward<GfxResPtr<GFX::Resource::IBindable>>(bind));
	}

	void BindingPass::BindAll(Graphics& gfx)
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