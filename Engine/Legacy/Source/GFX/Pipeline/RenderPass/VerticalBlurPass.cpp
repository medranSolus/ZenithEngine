#include "GFX/Pipeline/RenderPass/VerticalBlurPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	VerticalBlurPass::VerticalBlurPass(Graphics& gfx, std::string&& name)
		: RenderPass(std::forward<std::string>(name)),
		FullscreenPass(gfx, std::forward<std::string>(name))
	{
		AddBindableSink<Resource::RenderTargetShaderInput>("halfTarget");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("kernel");

		RegisterSink(Base::SinkDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("direction", direction));
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "BlurPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Normal));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Mask));
	}

	void VerticalBlurPass::Execute(Graphics& gfx)
	{
		direction->GetBuffer()["vertical"] = true;
		direction->Bind(gfx);
		FullscreenPass::Execute(gfx);
	}
}