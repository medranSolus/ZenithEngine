#include "GFX/Pipeline/RenderPass/SSAOBlurPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	SSAOBlurPass::SSAOBlurPass(Graphics& gfx, std::string&& name)
		: RenderPass(std::forward<std::string>(name)),
		FullscreenPass(gfx, std::forward<std::string>(name))
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
		ZE_DRAW_TAG_START(gfx, GetName());
		direction->GetBuffer()["vertical"] = !direction->GetBufferConst()["vertical"];
		direction->Bind(gfx);
		kernelBuffer->Bind(gfx);
		ssaoScratchBuffer->Unbind(gfx);
		BindAll(gfx);
		ssaoScratchBuffer->Bind(gfx);
		gfx.DrawIndexed(6);
		ZE_DRAW_TAG_END(gfx);
	}
}