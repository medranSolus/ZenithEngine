#include "LightCombinePass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	inline Data::CBuffer::DCBLayout LightCombinePass::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Color3, "ambientColor");
			initNeeded = false;
		}
		return layout;
	}

	LightCombinePass::LightCombinePass(Graphics& gfx, const std::string& name)
		: BindingPass(name), FullscreenPass(gfx, name)
	{
		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::IRenderTarget>("lightBuffer");
		AddBindableSink<Resource::IRenderTarget>("ssaoBuffer");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));

		ambientBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(LightCombinePass).name(), MakeLayout(), 9U);
		ambientBuffer->GetBuffer()["ambientColor"] = std::move(Data::ColorFloat3(0.05f, 0.05f, 0.05f));
		AddBind(ambientBuffer);
		AddBind(GFX::Resource::PixelShader::Get(gfx, "LightCombinePS"));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
	}

	void LightCombinePass::ShowWindow(Graphics& gfx)
	{
		Data::ColorFloat3 ambient = ambientBuffer->GetBufferConst()["ambientColor"];
		if (ImGui::ColorEdit3("Ambient color", reinterpret_cast<float*>(&ambient), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float))
			ambientBuffer->GetBuffer()["ambientColor"] = ambient;
	}
}