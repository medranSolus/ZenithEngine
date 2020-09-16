#include "LambertianPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	LambertianPass::LambertianPass(Graphics& gfx, const std::string& name) : QueuePass(name)
	{
		AddBindableSink<GFX::Resource::IBindable>("depthMap");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		shadowBuffer = std::make_shared<GFX::Resource::ConstBufferShadow>(gfx);
		AddBind(shadowBuffer);
		AddBind(GFX::Resource::ShadowSampler::Get(gfx));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthFirst));
	}

	void LambertianPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		shadowBuffer->Update(gfx);
		mainCamera->Bind(gfx);
		QueuePass::Execute(gfx);
	}
}