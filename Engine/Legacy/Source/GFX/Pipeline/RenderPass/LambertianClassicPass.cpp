#include "GFX/Pipeline/RenderPass/LambertianClassicPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	LambertianClassicPass::LambertianClassicPass(Graphics& gfx, std::string&& name)
		: RenderPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("geometryBuffer", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("geometryBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBindable<Resource::DepthStencil>::Make("depth", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_BACK));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None)); // Maybe other for transluscent objects and if so then add in material
	}

	void LambertianClassicPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		CullFrustum(*mainCamera);
		// For transparent surfaces
		SortBackFront(mainCamera->GetPos());
		mainCamera->BindCamera(gfx);
		QueuePass::Execute(gfx);
	}
}