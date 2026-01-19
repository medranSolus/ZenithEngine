#pragma once
#include "GFX/Resource/CBuffer.h"

namespace ZE::Data
{
#pragma pack(push, 1)
	// Component describing directional light params
	struct DirectionalLight
	{
		ColorF3 Color;
		float Intensity;
	};
	struct Direction { Float3 Direction; };

	// Component describing spot light params
	struct SpotLight
	{
		ColorF3 Color;
		float Intensity;
		Float3 Direction;
		float InnerAngle;
		float OuterAngle;
		float AttnLinear;
		float AttnQuad;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::Light::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};

	// Component containing point light parameters
	struct PointLight
	{
		ColorF3 Color;
		float Intensity;
		float AttnLinear;
		float AttnQuad;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::Light::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};
#pragma pack(pop)

	// Component containing data needed to render light
	struct LightBuffer
	{
		float Volume;
		GFX::Resource::CBuffer Buffer;
	};

	struct DirectionalLightBuffer { GFX::Resource::CBuffer Buffer; };
	struct SpotLightBuffer : public LightBuffer {};
	struct PointLightBuffer : public LightBuffer {};

	// Assure that all light components are registered as pools in data storage
	constexpr void InitLightComponents() noexcept;

#pragma region Functions
	constexpr void InitLightComponents() noexcept
	{
		Settings::AssureEntityPools<LightBuffer,
			DirectionalLight, Direction, DirectionalLightBuffer, 
			SpotLight, SpotLightBuffer,
			PointLight, PointLightBuffer>();
	}
#pragma endregion
}