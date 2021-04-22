#include "GFX/Pipeline/RenderPass/OutlineDrawBlurPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	OutlineDrawBlurPass::OutlineDrawBlurPass(Graphics& gfx, std::string&& name, U32 width, U32 height)
		: BindingPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
	{
		renderTarget = GfxResPtr<Resource::RenderTargetShaderInput>(gfx, width, height, 0);
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("blurTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "SolidPS"));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}

	void OutlineDrawBlurPass::Execute(Graphics& gfx)
	{
		renderTarget->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}