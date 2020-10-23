#include "DirectionalLight.h"

namespace GFX::Light
{
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
}