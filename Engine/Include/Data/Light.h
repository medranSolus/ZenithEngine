#pragma once
#include "GFX/Resource/CBuffer.h"

namespace ZE::Data
{
	// Component containing data needed to render light
	struct LightBuffer
	{
		float Volume;
		GFX::Resource::CBuffer Buffer;
	};

	// Component describing directional light params
	struct DirectionalLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
	};
	struct Direction { Float3 Direction; };
	struct DirectionalLightBuffer { GFX::Resource::CBuffer Buffer; };

	// Component describing spot light params
	struct SpotLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
		float InnerAngle;
		Float3 Direction;
		float OuterAngle;
		float AttnLinear;
		float AttnQuad;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};
	struct SpotLightBuffer : public LightBuffer {};

	// Component containing point light parameters
	struct PointLight
	{
		ColorF3 Color;
		float Intensity;
		ColorF3 Shadow;
		float AttnLinear;
		float AttnQuad;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};
	struct PointLightBuffer : public LightBuffer {};
}