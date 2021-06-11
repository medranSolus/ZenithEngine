#include "GFX/Pipeline/RenderPass/Base/BindingPass.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	void BindingPass::AddBind(GfxResPtr<GFX::Resource::IBindable>&& bind) noexcept
	{
		binds.emplace_back(std::forward<GfxResPtr<GFX::Resource::IBindable>>(bind));
	}

	void BindingPass::BindRenderAll(Graphics& gfx)
	{
		for (auto& bind : binds)
			bind->Bind(gfx);
	}

	void BindingPass::BindComputeAll(Graphics& gfx)
	{
		for (auto& bind : binds)
			bind->BindCompute(gfx);
	}
}