#include "OutlineDrawBlurPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	OutlineDrawBlurPass::OutlineDrawBlurPass(Graphics& gfx, const std::string& name, unsigned int width, unsigned int height)
		: BindingPass(name), QueuePass(name)
	{
		renderTarget = std::make_unique<Resource::RenderTargetShaderInput>(gfx, width, height, 0U);
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("blurTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "SolidPS"));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, false));
	}

	void OutlineDrawBlurPass::Execute(Graphics& gfx)
	{
		renderTarget->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}