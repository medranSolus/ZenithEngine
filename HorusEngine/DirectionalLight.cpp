#include "DirectionalLight.h"
#include "TechniqueFactory.h"

namespace GFX::Light
{
	DirectX::XMFLOAT3 DirectionalLight::dummyData = { 0.0f,0.0f,0.0f };

	inline Data::CBuffer::DCBLayout DirectionalLight::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Color3, "lightColor");
			layout.Add(DCBElementType::Float, "lightIntensity");
			layout.Add(DCBElementType::Color3, "shadowColor");
			layout.Add(DCBElementType::Float3, "direction");
			initNeeded = false;
		}
		return layout;
	}

	DirectionalLight::DirectionalLight(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& name,
		const DirectX::XMFLOAT3& direction, float intensity, const Data::ColorFloat3& color, float size)
		: name(name)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["lightColor"] = color;
		buffer["lightIntensity"] = intensity;
		buffer["shadowColor"] = std::move(Data::ColorFloat3(0.005f, 0.005f, 0.005f));
		buffer["direction"] = direction;
		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, typeid(DirectionalLight).name() + name, std::move(buffer));

		AddTechnique(gfx, Pipeline::TechniqueFactory::MakeDirectionalLighting(graph));
	}
}