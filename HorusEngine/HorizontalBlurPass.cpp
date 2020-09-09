#include "HorizontalBlurPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	HorizontalBlurPass::HorizontalBlurPass(Graphics& gfx, const std::string& name, unsigned int width, unsigned int height) : FullscreenPass(gfx, name)
	{
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("kernel");
		AddBindableSink<Resource::RenderTarget>("blurTarget");
		RegisterSink(Base::SinkDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("direction", direction));

		renderTarget = std::make_shared<Resource::RenderTargetShaderInput>(gfx, width, height, 0U);
		RegisterSource(Base::SourceDirectBindable<Resource::RenderTarget>::Make("halfTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "BlurPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, false));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Point, true));
	}

	void HorizontalBlurPass::Execute(Graphics& gfx)
	{
		direction->GetBuffer()["vertical"] = false;
		direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
	}
}