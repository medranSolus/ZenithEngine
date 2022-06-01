#include "GFX/Pipeline/RenderPass/LambertianDepthOptimizedPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"
#include "GFX/Pipeline/JobData.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	LambertianDepthOptimizedPass::LambertianDepthOptimizedPass(Graphics& gfx, std::string&& name)
		: RenderPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
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
		ZE_PERF_START("Lambertian");
		assert(mainCamera);
		CullFrustum(*mainCamera);
		SortFrontBack(mainCamera->GetPos());
		mainCamera->BindCamera(gfx);
		// Depth only pass
		depthOnlyPS->Bind(gfx);
		depthOnlyVS->Bind(gfx);
		depthOnlyStencilState->Bind(gfx);
		QueuePass::Execute(gfx, RenderChannel::Depth);
		// Normal pass
		ZE_DRAW_TAG_START(gfx, GetName() + "_final");
		lambertianStencilState->Bind(gfx);
		mainCamera->BindVS(gfx);
		for (auto& job : GetJobs())
		{
			ZE_DRAW_TAG_START(gfx, job.GetData().GetName());
			job.Execute(gfx, RenderChannel::All, 1);
			ZE_DRAW_TAG_END(gfx);
		}
		ZE_DRAW_TAG_END(gfx);
	}
}