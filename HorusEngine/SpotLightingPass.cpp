#include "SpotLightingPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Pipeline::RenderPass
{
	inline Data::CBuffer::DCBLayout SpotLightingPass::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Matrix, "shadowViewProjection");
			initNeeded = false;
		}
		return layout;
	}

	SpotLightingPass::SpotLightingPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name), shadowMapPass(gfx, "shadowMap", DirectX::XMMatrixPerspectiveFovLH(M_PI_2, 1.0f, 0.01f, 1000.0f))
	{
		AddBindableSink<Resource::IRenderTarget>("shadowMap");
		SetSinkLinkage("shadowMap", name + ".shadowMap.shadowMap");

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");

		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));

		shadowBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(ShadowMapPass).name(), MakeLayout(), 1U);
		AddBind(shadowBuffer);
		AddBind(GFX::Resource::PixelShader::Get(gfx, "SpotLightPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Light));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_FRONT, false));

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "LightVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, std::make_shared<Data::VertexLayout>(), vertexShader));
		AddBind(std::move(vertexShader));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	void SpotLightingPass::Reset() noexcept
	{
		shadowMapPass.Reset();
		QueuePass::Reset();
	}

	Base::BasePass& SpotLightingPass::GetInnerPass(std::deque<std::string> nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass.GetName())
			return shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void SpotLightingPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			shadowMapPass.BindLight(dynamic_cast<Light::ILight&>(job.GetData()));
			shadowMapPass.Execute(gfx);
			DirectX::XMStoreFloat4x4(&shadowBuffer->GetBuffer()["shadowViewProjection"], DirectX::XMMatrixTranspose(gfx.GetView() * gfx.GetProjection()));
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
		}
	}
}