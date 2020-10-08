#include "LightingPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	LightingPass::LightingPass(Graphics& gfx, const std::string& name)
		: BindingPass(name), QueuePass(name), FullscreenPass(gfx, name, "LightVS")
	{
		shadowMapPass = std::make_unique<ShadowMapPass>(gfx, "shadowMap");
		AddBindableSink<GFX::Resource::IBindable>("depthMap");
		SetSinkLinkage("depthMap", name + ".shadowMap.depthMap");

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		AddBind(GFX::Resource::NullGeometryShader::Get(gfx));
		AddBind(GFX::Resource::PixelShader::Get(gfx, "PointLightPS"));
		AddBind(GFX::Resource::Blender::Get(gfx, true));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthOff));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Anisotropic, false));
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
		mainCamera->BindPS(gfx);
		for (auto& job : GetJobs())
		{
			auto& light = dynamic_cast<Light::ILight&>(job.GetData());
			shadowMapPass->BindLight(light);
			shadowMapPass->Execute(gfx);
			mainCamera->BindCamera(gfx);
			BindAll(gfx);
			job.Execute(gfx);
		}
	}
}