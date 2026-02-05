#include "MathLight.h"

namespace ZE::Math::Light
{
	float GetLightVolume(const ColorF3& color, float intensity, float attnLinear, float attnQuad) noexcept
	{
		const float lightMax = intensity * std::fmaxf(std::fmaxf(color.RGB.x, color.RGB.y), color.RGB.z);
		return (-attnLinear + std::sqrtf(attnLinear * attnLinear - 4.0f * attnQuad * (1.0f - lightMax * 256.0f))) / (2.0f * attnQuad);
	}

	float ApplyPQ(float linearColor, float exponentScaleFactor) noexcept
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

	Vector ImportanceSampleGGX(const Float2& Xi, float roughness, Vector N) noexcept
	{
		Vector up = XMVectorGetZ(XMVectorAbs(N)) < 0.999f ? XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) : XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		Vector tan = XMVector3Normalize(XMVector3Cross(up, N));
		Vector bitan = XMVector3Cross(N, tan);

		return ImportanceSampleGGX(Xi, roughness, N, tan, bitan);
	}

	Vector ImportanceSampleGGX(const Float2& Xi, float roughness, Vector N, Vector tan, Vector bitan) noexcept
	{
		const float a = roughness * roughness;

		const float phi = PI2 * Xi.x;
		const float cosTheta = std::sqrtf((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
		const float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);

		// From spherical coords to cartesian coords
		const float H_x = std::cosf(phi) * sinTheta;
		const float H_y = std::sinf(phi) * sinTheta;

		// From tangent-space vector to world-space sample vector
		// sample vector == tan * H_x + bitan * H_y + N * H_z (cosTheta)
		return XMVector3Normalize(XMVectorAdd(XMVectorMultiply(N, XMVectorReplicate(cosTheta)),
			XMVectorAdd(XMVectorMultiply(tan, XMVectorReplicate(H_x)),
				XMVectorMultiply(bitan, XMVectorReplicate(H_y)))));
	}

	Float2 IntegrateBRDF(float NoV, float roughness, U32 samples) noexcept
	{
		float A = 0.0f;
		float B = 0.0f;

		Vector V = XMVectorSet(std::sqrt(1.0f - NoV * NoV), 0.0f, NoV, 0.0f);
		Vector N = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		Vector up = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		Vector tan = XMVector3Normalize(XMVector3Cross(up, N));
		Vector bitan = XMVector3Cross(N, tan);

		for (U32 i = 0; i < samples; ++i)
		{
			Float2 Xi = HammersleySequence(i, samples);
			Vector H = ImportanceSampleGGX(Xi, roughness, N, tan, bitan);
			Vector L = XMVector3Normalize(XMVectorSubtract(XMVectorMultiply(XMVectorMultiply(XMVectorReplicate(2.0f), XMVector3Dot(V, H)), H), V));

			const float NdotL = std::max(XMVectorGetZ(L), 0.0f);

			if (NdotL > 0.0f)
			{
				const float NdotH = std::max(XMVectorGetZ(H), 0.0f);
				const float VdotH = std::max(XMVectorGetX(XMVector3Dot(V, H)), 0.0f);
				const float NdotV = std::max(XMVectorGetX(XMVector3Dot(N, V)), 0.0f);
				const float G = SelfShadowingSmithSchlick<true>(roughness, NdotV, NdotL);

				const float G_Vis = (G * VdotH) / (NdotH * NdotV);
				const float Fc = std::pow(1.0f - VdotH, 5.0f);

				A += (1.0f - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}
		return { A / static_cast<float>(samples), B / static_cast<float>(samples) };
	}
}