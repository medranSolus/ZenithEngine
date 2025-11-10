#pragma once
#include "MathExt.h"

namespace ZE::Math::Light
{
	inline float GetLightVolume(const ColorF3& color, float intensity, float attnLinear, float attnQuad) noexcept
	{
		const float lightMax = intensity * std::fmaxf(std::fmaxf(color.RGB.x, color.RGB.y), color.RGB.z);
		return (-attnLinear + std::sqrtf(attnLinear * attnLinear - 4.0f * attnQuad * (1.0f - lightMax * 256.0f))) / (2.0f * attnQuad);
	}

	constexpr void SetLightAttenuation(float& linear, float& quad, U64 range) noexcept
	{
		linear = 4.5f / static_cast<float>(range);
		quad = 75.0f / static_cast<float>(range * range);
	}

	constexpr float RadicalInverse_VdC(U32 bits) noexcept
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return static_cast<float>(static_cast<double>(bits) * 2.3283064365386963e-10);
	}

	constexpr Float2 Hammersley(U32 i, U32 N) noexcept
	{
		return { static_cast<float>(i) / static_cast<float>(N), RadicalInverse_VdC(i) };
	}

	constexpr float GeometrySchlickGGX(float NdotV, float roughnessRemapped) noexcept
	{
		return NdotV / (NdotV * (1.0f - roughnessRemapped) + roughnessRemapped);
	}

	template<bool IBL>
	constexpr float SelfShadowingSmithSchlick(float roughness, float NdotV, float NdotL) noexcept
	{
		float roughnessRemapped;
		if constexpr (IBL)
			roughnessRemapped = roughness * roughness / 2.0f;
		else
		{
			roughnessRemapped = roughness + 1.0f;
			roughnessRemapped = roughnessRemapped * roughnessRemapped / 8.0f;
		}

		return GeometrySchlickGGX(NdotV, roughnessRemapped) * GeometrySchlickGGX(NdotL, roughnessRemapped);
	}

	Vector ImportanceSampleGGX(const Float2& Xi, float roughness, Vector N, Vector tan, Vector bitan) noexcept;
	Float2 IntegrateBRDF(float NdotV, float roughness, U32 samples) noexcept;
}