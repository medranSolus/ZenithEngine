#include "LambertianClassicPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	LambertianClassicPass::LambertianClassicPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name)
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
		mainCamera->BindCamera(gfx);
		// For transparent surfaces
		SortBackFront(mainCamera->GetPos());
		QueuePass::Execute(gfx);
	}
}