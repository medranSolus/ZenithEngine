#include "GFX/Pipeline/RenderPass/HorizontalBlurPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	HorizontalBlurPass::HorizontalBlurPass(Graphics& gfx, std::string&& name, U32 width, U32 height)
		: RenderPass(std::forward<std::string>(name)), FullscreenPass(gfx, std::forward<std::string>(name))
	{
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("kernel");
		AddBindableSink<Resource::RenderTarget>("blurTarget");
		RegisterSink(Base::SinkDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("direction", direction));

		renderTarget = GfxResPtr<Resource::RenderTargetShaderInput>(gfx, width, height, 0);
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("halfTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "BlurPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}

	void HorizontalBlurPass::Execute(Graphics& gfx)
	{
		direction->GetBuffer()["vertical"] = false;
		direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
	}
}