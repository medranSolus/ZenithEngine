#include "RenderCommander.h"
#include "GfxResources.h"

namespace GFX::Pipeline
{
	void RenderCommander::Render(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		Resource::DepthStencil::Get(gfx, Resource::DepthStencil::StencilMode::Off)->Bind(gfx);
		passes.at(0).Execute(gfx);
	}

	void RenderCommander::Reset() noexcept
	{
		for (auto& pass : passes)
			pass.Reset();
	}
}