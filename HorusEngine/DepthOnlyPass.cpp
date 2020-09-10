#include "DepthOnlyPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	DepthOnlyPass::DepthOnlyPass(Graphics& gfx, const std::string& name) : QueuePass(name)
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::NullPixelShader::Get(gfx));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, true));
	}

	void DepthOnlyPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->Bind(gfx);
		QueuePass::Execute(gfx);
	}
}