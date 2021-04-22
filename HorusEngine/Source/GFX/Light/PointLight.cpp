#include "GFX/Light/PointLight.h"
#include "GFX/Light/Volume/GlobeVolume.h"
#include "GFX/Shape/SolidGlobe.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace GFX::Light
{
	Data::CBuffer::DCBLayout PointLight::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Color3, "lightColor");
			layout.Add(DCBElementType::Float, "lightIntensity");
			layout.Add(DCBElementType::Color3, "shadowColor");
			layout.Add(DCBElementType::Float, "atteuationLinear");
			layout.Add(DCBElementType::Float3, "lightPos");
			layout.Add(DCBElementType::Float, "attenuationQuad");
			initNeeded = false;
		}
		return layout;
	}

	PointLight::PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name, float intensity,
		const ColorF3& color, const Float3& position, U64 range, float radius)
		: range(range)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["lightIntensity"] = intensity;
		buffer["lightColor"] = color;
		buffer["lightPos"] = position;
		buffer["shadowColor"] = ColorF3(0.005f, 0.005f, 0.005f);

		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, "P_" + name, std::move(buffer));
		mesh = std::make_unique<Shape::SolidGlobe>(gfx, graph, position, std::move(name), buffer["lightColor"], 4, 4, radius, radius, radius);
		volume = std::make_unique<Volume::GlobeVolume>(gfx, 3);

		SetAttenuation(range);
		volume->Update(lightBuffer->GetBufferConst());
		AddTechnique(gfx, Pipeline::TechniqueFactory::MakePointLighting(graph));
	}

	bool PointLight::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		static const U64 STEP = 1;
		ImGui::Columns(2, "##point_light", false);
		if (ImGui::InputScalar("Range", ImGuiDataType_U64, &range, &STEP))
			SetAttenuation(range);
		ImGui::NextColumn();
		if (ILight::Accept(gfx, probe))
		{
			lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
			volume->Update(lightBuffer->GetBufferConst());
			return true;
		}
		return false;
	}
}