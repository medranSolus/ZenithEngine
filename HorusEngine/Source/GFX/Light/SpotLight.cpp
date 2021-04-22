#include "GFX/Light/SpotLight.h"
#include "GFX/Light/Volume/ConeVolume.h"
#include "GFX/Shape/SolidCone.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace GFX::Light
{
	Data::CBuffer::DCBLayout SpotLight::MakeLayout() noexcept
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
			layout.Add(DCBElementType::Float3, "direction");
			layout.Add(DCBElementType::Float, "innerAngle");
			layout.Add(DCBElementType::Float, "outerAngle");
			initNeeded = false;
		}
		return layout;
	}

	SpotLight::SpotLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name, float intensity,
		const ColorF3& color, const Float3& position, U64 range, float size,
		float innerAngle, float outerAngle, const Float3& direction)
		: range(range)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["lightIntensity"] = intensity;
		buffer["lightColor"] = color;
		buffer["lightPos"] = position;
		buffer["shadowColor"] = ColorF3(0.005f, 0.005f, 0.005f);
		buffer["direction"] = direction;
		buffer["innerAngle"] = innerAngle;
		buffer["outerAngle"] = outerAngle;

		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, "S_" + name, std::move(buffer));
		mesh = std::make_unique<Shape::SolidCone>(gfx, graph, position, std::move(name), buffer["lightColor"], 8, size);
		volume = std::make_unique<Volume::ConeVolume>(gfx, 8);

		SetAttenuation(range);
		volume->Update(lightBuffer->GetBufferConst());
		AddTechnique(gfx, Pipeline::TechniqueFactory::MakeSpotLighting(graph));
	}

	bool SpotLight::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		static const U64 STEP = 1;
		ImGui::Columns(2, "##spot_light", false);
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