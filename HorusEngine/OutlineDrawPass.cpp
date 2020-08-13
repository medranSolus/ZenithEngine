#include "OutlineDrawPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	OutlineDrawPass::OutlineDrawPass(Graphics& gfx, const std::string& name)
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::PixelShader::Get(gfx, "SolidPS.cso"));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, false));
	}
}