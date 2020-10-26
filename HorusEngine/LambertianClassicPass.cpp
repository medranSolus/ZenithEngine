#include "LambertianClassicPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"
#include "JobData.h"

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
		// Back-front sort for transparent surfaces
		//const DirectX::XMVECTOR cameraPos = DirectX::XMLoadFloat3(&mainCamera->GetPos());
		//std::sort(GetJobs().begin(), GetJobs().end(), [&cameraPos](const Job& j1, const Job& j2)
		//	{
		//		const float len1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j1.GetStep().GetTransformPos()), cameraPos)));
		//		const float len2 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j2.GetStep().GetTransformPos()), cameraPos)));
		//		return len1 > len2;
		//	});
		mainCamera->BindCamera(gfx);
		mainCamera->BindVS(gfx);
		QueuePass::Execute(gfx);
	}
}