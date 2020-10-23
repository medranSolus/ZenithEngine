#include "PointLightingPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	PointLightingPass::PointLightingPass(Graphics& gfx, const std::string& name, UINT shadowMapSize)
		: QueuePass(name), shadowMapPass(gfx, "shadowMap", shadowMapSize)
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowMap");
		SetSinkLinkage("shadowMap", name + ".shadowMap.shadowMap");

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");

		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));

		AddBind(GFX::Resource::NullGeometryShader::Get(gfx));
		AddBind(GFX::Resource::PixelShader::Get(gfx, "PointLightPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Light));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_FRONT, false));

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "LightVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, std::make_shared<Data::VertexLayout>(), vertexShader));
		AddBind(std::move(vertexShader));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	void PointLightingPass::Reset() noexcept
	{
		shadowMapPass.Reset();
		QueuePass::Reset();
	}

	Base::BasePass& PointLightingPass::GetInnerPass(std::deque<std::string> nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass.GetName())
			return shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void PointLightingPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			shadowMapPass.BindLight(dynamic_cast<Light::ILight&>(job.GetData()));
			shadowMapPass.Execute(gfx);
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
		}
	}
}