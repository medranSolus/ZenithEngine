#include "LightingPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	LightingPass::LightingPass(Graphics& gfx, const std::string& name)
		: QueuePass(name)
	{
		shadowMapPass = std::make_unique<ShadowMapPass>(gfx, "shadowMap");
		AddBindableSink<GFX::Resource::IBindable>("shadowMap");
		SetSinkLinkage("shadowMap", name + ".shadowMap.shadowMap");

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");

		renderTarget = Resource::RenderTargetEx::Get(gfx, 9U, { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT });
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));

		AddBind(GFX::Resource::NullGeometryShader::Get(gfx));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Light));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Anisotropic, true, 0U));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Point, true, 1U));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_FRONT));

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "LightVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, std::make_shared<Data::VertexLayout>(), vertexShader));
		AddBind(std::move(vertexShader));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	void LightingPass::Reset() noexcept
	{
		shadowMapPass->Reset();
		QueuePass::Reset();
	}

	Base::BasePass& LightingPass::GetInnerPass(std::deque<std::string> nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass->GetName())
			return *shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void LightingPass::Execute(Graphics& gfx)
	{
		renderTarget->Clear(gfx, { 0.0f, 0.0f, 0.0f, 0.0f });
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			shadowMapPass->BindLight(dynamic_cast<Light::ILight&>(job.GetData()));
			shadowMapPass->Execute(gfx);
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
		}
	}
}