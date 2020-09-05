#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name) : QueuePass(name)
	{
		depthStencil = std::make_shared<Resource::DepthStencilShaderInput>(gfx, 3U, Resource::DepthStencil::Usage::ShadowDepth);
		RegisterSource(Base::SourceDirectBindable<Resource::DepthStencil>::Make("depthMap", depthStencil));

		AddBind(GFX::Resource::NullPixelShader::Get(gfx));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, false));
	}

	void ShadowMapPass::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		assert(shadowCamera);
		shadowCamera->Bind(gfx);
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}