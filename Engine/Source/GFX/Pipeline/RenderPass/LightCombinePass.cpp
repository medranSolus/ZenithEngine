#include "GFX/Pipeline/RenderPass/LightCombinePass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Pipeline/Resource/PipelineResources.h"
#include "GFX/Resource/GfxResources.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	Data::CBuffer::DCBLayout LightCombinePass::MakeLayout() noexcept
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

	LightCombinePass::LightCombinePass(Graphics& gfx, std::string&& name)
		: RenderPass(std::forward<std::string>(name)), FullscreenPass(gfx, std::forward<std::string>(name))
	{
		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::IRenderTarget>("lightBuffer");
		AddBindableSink<Resource::IRenderTarget>("ssaoBuffer");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("sceneTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("sceneTarget", renderTarget));

		ambientBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, "$L", MakeLayout(), 13);
		ambientBuffer->GetBuffer()["ambientColor"] = ColorF3(0.05f, 0.05f, 0.05f);
		AddBind(ambientBuffer);
		AddBind(GFX::Resource::PixelShader::Get(gfx, "LightCombinePS"));
	}

	void LightCombinePass::ShowWindow(Graphics& gfx)
	{
		ColorF3 ambient = ambientBuffer->GetBufferConst()["ambientColor"];
		ImGui::Text("Ambient color");
		ImGui::SetNextItemWidth(-5.0f);
		if (ImGui::ColorEdit3("##ambient_color", reinterpret_cast<float*>(&ambient),
			ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoLabel))
			ambientBuffer->GetBuffer()["ambientColor"] = ambient;
	}
}