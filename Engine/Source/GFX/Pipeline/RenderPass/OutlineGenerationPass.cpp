#include "GFX/Pipeline/RenderPass/OutlineGenerationPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	OutlineGenerationPass::OutlineGenerationPass(Graphics& gfx, std::string&& name)
		: BindingPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::NullPixelShader::Get(gfx));
		AddBind(GFX::Resource::VertexShader::Get(gfx, "SolidVS"));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Write));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_NONE));
	}
}