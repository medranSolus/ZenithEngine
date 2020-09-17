#include "PointLight.h"
#include "SolidGlobe.h"

namespace GFX::Light
{
	Data::CBuffer::DCBLayout PointLight::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Color3, "ambientColor");
			layout.Add(DCBElementType::Float, "atteuationConst");
			layout.Add(DCBElementType::Color3, "lightColor");
			layout.Add(DCBElementType::Float, "atteuationLinear");
			layout.Add(DCBElementType::Float3, "lightPos");
			layout.Add(DCBElementType::Float, "attenuationQuad");
			layout.Add(DCBElementType::Color3, "shadowColor");
			layout.Add(DCBElementType::Float, "lightIntensity");
			initNeeded = false;
		}
		return layout;
	}

	PointLight::PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, float radius)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["ambientColor"] = std::move(Data::ColorFloat3(0.05f, 0.05f, 0.05f));
		buffer["atteuationConst"] = 1.0f;
		buffer["lightColor"] = std::move(Data::ColorFloat3(1.0f, 1.0f, 1.0f));
		buffer["atteuationLinear"] = 0.045f;
		buffer["lightPos"] = position;
		buffer["attenuationQuad"] = 0.0075f;
		buffer["shadowColor"] = std::move(Data::ColorFloat3(0.005f, 0.005f, 0.005f));
		buffer["lightIntensity"] = 5.0f;
		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, name, std::move(buffer));
		mesh = std::make_shared<Shape::SolidGlobe>(gfx, graph, position, name, buffer["lightColor"], 3, 3, radius, radius, radius);

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		auto technique = std::make_shared<Pipeline::Technique>("Lighting", RenderChannel::Light);
		technique->AddStep({ graph, "lighting" });
		techniques.emplace_back(std::move(technique));
		SetTechniques(gfx, std::move(techniques), *mesh);
	}

	void PointLight::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		lightBuffer->Accept(gfx, probe);
		BaseLight::Accept(gfx, probe);
	}

	void PointLight::Bind(Graphics& gfx)
	{
		lightBuffer->GetBuffer()["lightPos"] = mesh->GetPos();
		lightBuffer->Bind(gfx);
	}
}