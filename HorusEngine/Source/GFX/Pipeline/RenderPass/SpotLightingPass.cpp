#include "GFX/Pipeline/RenderPass/SpotLightingPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	Data::CBuffer::DCBLayout SpotLightingPass::MakeLayout() noexcept
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

	SpotLightingPass::SpotLightingPass(Graphics& gfx, std::string&& name)
		: BindingPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name)),
		shadowMapPass(gfx, "shadowMap",
			Math::XMMatrixPerspectiveFovLH(static_cast<float>(M_PI_2), 1.0f, 0.01f, 1000.0f))
	{
		AddBindableSink<Resource::IRenderTarget>("shadowMap");
		SetSinkLinkage("shadowMap", GetName() + ".shadowMap.shadowMap");

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");

		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("lightBuffer", renderTarget));

		shadowBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, "$SM", MakeLayout(), 1);
		AddBind(shadowBuffer);
		AddBind(GFX::Resource::PixelShader::Get(gfx, "SpotLightPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Light));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_FRONT, false));

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "LightVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, std::make_shared<Data::VertexLayout>(), vertexShader));
		AddBind(std::move(vertexShader));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	void SpotLightingPass::Reset() noexcept
	{
		shadowMapPass.Reset();
		QueuePass::Reset();
	}

	Base::BasePass& SpotLightingPass::GetInnerPass(const std::deque<std::string>& nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass.GetName())
			return shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void SpotLightingPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		DRAW_TAG_START(gfx, GetName());
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			DRAW_TAG_START(gfx, job.GetData().GetName());
			shadowMapPass.BindLight(dynamic_cast<const Light::ILight&>(job.GetData()));
			shadowMapPass.Execute(gfx);
			Math::XMStoreFloat4x4(&shadowBuffer->GetBuffer()["shadowViewProjection"],
				Math::XMMatrixTranspose(gfx.GetView() * gfx.GetProjection()));
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
			DRAW_TAG_END(gfx);
		}
		DRAW_TAG_END(gfx);
	}
}