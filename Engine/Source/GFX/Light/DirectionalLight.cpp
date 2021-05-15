#include "GFX/Light/DirectionalLight.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace ZE::GFX::Light
{
	Data::CBuffer::DCBLayout DirectionalLight::MakeLayout() noexcept
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

	DirectionalLight::DirectionalLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name,
		float intensity, const ColorF3& color, const Float3& direction)
		: name(std::forward<std::string>(name))
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["lightColor"] = color;
		buffer["lightIntensity"] = intensity;
		buffer["shadowColor"] = ColorF3(0.005f, 0.005f, 0.005f);
		buffer["direction"] = direction;
		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, "D_" + GetName(), std::move(buffer));

		AddTechnique(gfx, Pipeline::TechniqueFactory::MakeDirectionalLighting(graph));
	}
}