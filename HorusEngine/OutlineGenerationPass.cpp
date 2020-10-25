#include "OutlineGenerationPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	OutlineGenerationPass::OutlineGenerationPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name)
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::NullPixelShader::Get(gfx));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Write));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_NONE));
	}
}