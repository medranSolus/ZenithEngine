#include "GFX/Pipeline/RenderPass/HDRGammaCorrectionPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	HDRGammaCorrectionPass::HDRGammaCorrectionPass(Graphics& gfx, std::string&& name)
		: BindingPass(std::forward<std::string>(name)),
		FullscreenPass(gfx, std::forward<std::string>(name))
	{
		AddBindableSink<Resource::RenderTargetShaderInput>("scene");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "HDRGammaPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}
}