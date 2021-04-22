#include "GFX/Pipeline/RenderPass/DirectionalLightingPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	Data::CBuffer::DCBLayout DirectionalLightingPass::MakeLayout() noexcept
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

	DirectionalLightingPass::DirectionalLightingPass(Graphics& gfx, std::string&& name, U32 mapSize)
		: BindingPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name)),
		FullscreenPass(gfx, std::forward<std::string>(name)), shadowMapPass(gfx, "shadowMap",
			Math::XMMatrixOrthographicLH(static_cast<float>(mapSize), static_cast<float>(mapSize), 0.01f, 1000.0f))
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
		AddBind(GFX::Resource::PixelShader::Get(gfx, "DirectionalLightPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::Light));
	}

	void DirectionalLightingPass::Reset() noexcept
	{
		shadowMapPass.Reset();
		QueuePass::Reset();
	}

	Base::BasePass& DirectionalLightingPass::GetInnerPass(const std::deque<std::string>& nameChain)
	{
		if (nameChain.size() == 1 && nameChain.front() == shadowMapPass.GetName())
			return shadowMapPass;
		throw RGC_EXCEPT("Wrong inner pass name: " + nameChain.front());
	}

	void DirectionalLightingPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		DRAW_TAG_START(gfx, GetName());
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			DRAW_TAG_START(gfx, job.GetData().GetName());
			//shadowMapPass.BindLight(dynamic_cast<Light::ILight&>(job.GetData()));
			//shadowMapPass.Execute(gfx);
			//Math::XMStoreFloat4x4(&shadowBuffer->GetBuffer()["shadowViewProjection"], Math::XMMatrixTranspose(gfx.GetView() * gfx.GetProjection()));
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
			DRAW_TAG_END(gfx);
		}
		DRAW_TAG_END(gfx);
	}
}