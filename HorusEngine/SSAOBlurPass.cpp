#include "SSAOBlurPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	SSAOBlurPass::SSAOBlurPass(Graphics& gfx, const std::string& name)
		: FullscreenPass(gfx, name)
	{
		RegisterSink(Base::SinkDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("direction", direction));
		RegisterSink(Base::SinkDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("ssaoKernel", kernelBuffer));
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("ssaoTarget", renderTarget));
		RegisterSink(Base::SinkDirectBindable<Resource::IRenderTarget>::Make("ssaoBuffer", ssaoScratchBuffer));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("ssaoScratch", ssaoScratchBuffer));
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("ssaoBuffer", renderTarget));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "SSAOBlurPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}

	void SSAOBlurPass::Execute(Graphics& gfx)
	{
		direction->GetBuffer()["vertical"] = !direction->GetBufferConst()["vertical"];
		direction->Bind(gfx);
		kernelBuffer->Bind(gfx);
		BindAll(gfx);
		ssaoScratchBuffer->Bind(gfx);
		gfx.DrawIndexed(6U);
	}
}