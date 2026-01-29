#pragma once
#include "MathExt.h"

namespace ZE::Math::Light
{
	// 1 in the linear frame-buffer space corresponds to this value of physical luminance [cd/m^2]
	constexpr float REFERENCE_LUMINANCE = 100.0f;

	inline float GetLightVolume(const ColorF3& color, float intensity, float attnLinear, float attnQuad) noexcept
	{
		const float lightMax = intensity * std::fmaxf(std::fmaxf(color.RGB.x, color.RGB.y), color.RGB.z);
		return (-attnLinear + std::sqrtf(attnLinear * attnLinear - 4.0f * attnQuad * (1.0f - lightMax * 256.0f))) / (2.0f * attnQuad);
	}

	inline float ApplyPQ(float linearColor, float exponentScaleFactor) noexcept
	{
		constexpr float PQ_M1 = 2610.0f / 16384.0f;
		constexpr float PQ_M2 = 2523.0f / 32.0f;
		constexpr float PQ_C1 = 3424.0f / 4096.0f;
		constexpr float PQ_C2 = 2413.0f / 128.0f;
		constexpr float PQ_C3 = 2392.0f / 128.0f;
		constexpr float PQ_PQC = 10000.0f; // Max supported luminance

		linearColor *= REFERENCE_LUMINANCE;
		if (linearColor < 0.0f)
			linearColor = 0.0f;
		else if (linearColor > PQ_PQC)
			linearColor = 1.0f;
		else
			linearColor /= PQ_PQC;

		float y = std::powf(linearColor, PQ_M1);
		return std::exp2f(exponentScaleFactor * PQ_M2 * (std::log2f(PQ_C1 + PQ_C2 * y) - std::log2f(1.0f + PQ_C3 * y)));
	}

	constexpr void SetLightAttenuation(float& linear, float& quad, U64 range) noexcept
	{
		linear = 4.5f / static_cast<float>(range);
		quad = 75.0f / static_cast<float>(range * range);
	}

	// Van der Corput radical inverse sequence
	constexpr float RadicalInverse_VdC(U32 bits) noexcept
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return static_cast<float>(static_cast<double>(bits) * 2.3283064365386963e-10);
	}

	constexpr Float2 HammersleySequence(U32 i, U32 N) noexcept
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
			roughnessRemapped = roughness * roughness * 0.5f;
		else
		{
			roughnessRemapped = roughness + 1.0f;
			roughnessRemapped = roughnessRemapped * roughnessRemapped * 0.125f;
		}

		return GeometrySchlickGGX(NdotV, roughnessRemapped) * GeometrySchlickGGX(NdotL, roughnessRemapped);
	}

	Vector ImportanceSampleGGX(const Float2& Xi, float roughness, Vector N) noexcept;
	Vector ImportanceSampleGGX(const Float2& Xi, float roughness, Vector N, Vector tan, Vector bitan) noexcept;
	Float2 IntegrateBRDF(float NdotV, float roughness, U32 samples) noexcept;
}