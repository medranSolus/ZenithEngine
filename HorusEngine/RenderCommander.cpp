#include "RenderCommander.h"
#include "GfxResources.h"

namespace GFX::Pipeline
{
	void RenderCommander::Render(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		depthStencil.Clear(gfx);
		gfx.BindSwapBuffer(depthStencil);
		// Classic render pass
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Off)->Bind(gfx);
		passes.at(0).Execute(gfx);
		// Draw to stencil buffer
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Write)->Bind(gfx);
		Resource::NullPixelShader::Get(gfx)->Bind(gfx);
		passes.at(1).Execute(gfx);
		// Mask render target with stencil buffer
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Mask)->Bind(gfx);
		Resource::PixelShader::Get(gfx, "SolidPS.cso")->Bind(gfx);
		passes.at(2).Execute(gfx);
	}

	void RenderCommander::Reset() noexcept
	{
		for (auto& pass : passes)
			pass.Reset();
	}
}