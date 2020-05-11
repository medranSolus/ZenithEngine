#include "RenderCommander.h"
#include "GfxResources.h"

namespace GFX::Pipeline
{
	void RenderCommander::Render(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		// Classic render pass
		Resource::DepthStencil::Get(gfx, Resource::DepthStencil::StencilMode::Off)->Bind(gfx);
		passes.at(0).Execute(gfx);
		//// Draw to stencil buffer
		//Resource::DepthStencil::Get(gfx, Resource::DepthStencil::StencilMode::Write)->Bind(gfx);
		//Resource::NullPixelShader::Get(gfx)->Bind(gfx);
		//passes.at(1).Execute(gfx);
		//// Mask render target with stencil buffer
		//Resource::DepthStencil::Get(gfx, Resource::DepthStencil::StencilMode::Mask)->Bind(gfx);
		//Resource::PixelShader::Get(gfx, "SolidPS.cso")->Bind(gfx);
		//passes.at(2).Execute(gfx);
	}

	void RenderCommander::Reset() noexcept
	{
		for (auto& pass : passes)
			pass.Reset();
	}
}