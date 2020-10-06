#include "GammaCorrectionPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	GammaCorrectionPass::GammaCorrectionPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), FullscreenPass(gfx, name)
	{
		AddBindableSink<Resource::RenderTargetShaderInput>("scene");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "GammaPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, false));
	}
}