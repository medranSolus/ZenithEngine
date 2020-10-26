#include "LambertianDepthOptimizedPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"
#include "JobData.h"

namespace GFX::Pipeline::RenderPass
{
	LambertianDepthOptimizedPass::LambertianDepthOptimizedPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name)
	{
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("geometryBuffer", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("geometryBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		depthOnlyPS = GFX::Resource::NullPixelShader::Get(gfx);
		depthOnlyVS = GFX::Resource::VertexShader::Get(gfx, "SolidVS");
		depthOnlyStencilState = GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off);
		lambertianStencilState = GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthFirst);

		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_BACK));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}

	void LambertianDepthOptimizedPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->BindCamera(gfx);
		// Early front-back sort
		const DirectX::XMVECTOR cameraPos = DirectX::XMLoadFloat3(&mainCamera->GetPos());
		std::sort(GetJobs().begin(), GetJobs().end(), [&cameraPos](const Job& j1, const Job& j2)
			{
				const float len1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j1.GetStep().GetTransformPos()), cameraPos)));
				const float len2 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&j2.GetStep().GetTransformPos()), cameraPos)));
				return len1 < len2;
			});
		// Depth only pass
		depthOnlyPS->Bind(gfx);
		depthOnlyVS->Bind(gfx);
		depthOnlyStencilState->Bind(gfx);
		QueuePass::Execute(gfx, RenderChannel::Depth);
		// Normal pass
		lambertianStencilState->Bind(gfx);
		for (auto& job : GetJobs())
			job.Execute(gfx);
	}
}