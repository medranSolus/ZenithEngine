#pragma once
#include "GFX/Resource/CBuffer.h"

namespace ZE::Data
{
#pragma pack(push, 1)
	// Component describing directional light params
	struct DirectionalLight
	{
		ColorF3 Color = {};
		// Radiant flux in Watts together with color
		float Intensity = 0.0f;
	};
	struct Direction { Float3 Dir = {}; };

	// Component describing spot light params
	struct SpotLight
	{
		ColorF3 Color = {};
		// Radiant flux in Watts together with color
		float Intensity = 0.0f;
		Float3 Direction = {};
		float InnerAngle = 0.0f;
		float OuterAngle = 0.0f;
		float AttnLinear = 0.0f;
		float AttnQuad = 0.0f;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::Light::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};

	// Component containing point light parameters
	struct PointLight
	{
		ColorF3 Color = {};
		// Radiant flux in Watts together with color
		float Intensity = 0.0f;
		float AttnLinear = 0.0f;
		float AttnQuad = 0.0f;

		constexpr void SetAttenuationRange(U64 range) noexcept { Math::Light::SetLightAttenuation(AttnLinear, AttnQuad, range); }
	};
#pragma pack(pop)

	// Component containing data needed to render light
	struct LightBuffer
	{
		float Volume = 0.0f;
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