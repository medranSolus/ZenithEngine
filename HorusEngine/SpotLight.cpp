#include "SpotLight.h"
#include "ConeVolume.h"
#include "SolidCone.h"
#include "TechniqueFactory.h"

namespace GFX::Light
{
	inline Data::CBuffer::DCBLayout SpotLight::MakeLayout() noexcept
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

	SpotLight::SpotLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, size_t range,
		float innerAngle, float outerAngle, const DirectX::XMFLOAT3& direction, float intensity, const Data::ColorFloat3& color, float size)
		: range(range)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["lightIntensity"] = intensity;
		buffer["lightColor"] = color;
		buffer["lightPos"] = position;
		buffer["shadowColor"] = std::move(Data::ColorFloat3(0.005f, 0.005f, 0.005f));
		buffer["direction"] = direction;
		buffer["innerAngle"] = static_cast<float>(M_PI - FLT_EPSILON) * innerAngle / 180.0f;
		buffer["outerAngle"] = static_cast<float>(M_PI - FLT_EPSILON) * outerAngle / 180.0f;

		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, typeid(SpotLight).name() + name, std::move(buffer));
		mesh = std::make_unique<Shape::SolidCone>(gfx, graph, position, name, buffer["lightColor"], 8, size);
		volume = std::make_unique<Volume::ConeVolume>(gfx, 8);

		SetAttenuation(range);
		volume->Update(lightBuffer->GetBufferConst());
		AddTechnique(gfx, Pipeline::TechniqueFactory::MakeSpotLighting(graph));
	}

	bool SpotLight::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		static const size_t STEP = 1;
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