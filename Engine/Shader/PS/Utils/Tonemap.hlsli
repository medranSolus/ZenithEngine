#ifndef TONEMAP_UTILS_HLSLI
#define TONEMAP_UTILS_HLSLI

float GetLuma(float3 color)
{
	return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

// Reinhard tone mapping (favor for bright areas)
float3 GetReinhard(const in float3 color, const in float exposure, const in float offset)
{
	return (color * exposure) / (offset + color / exposure);
}

float3 GetReinhardLuma(const in float3 color, const in float exposure, const in float offset)
{
	return color * (exposure / (offset + GetLuma(color) / exposure));
}

float3 GetReinhardJodie(const in float3 color, const in float exposure, const in float offset)
{
	const float3 base = GetReinhard(color, exposure, offset);
	const float3 luma = GetReinhardLuma(color, exposure, offset);
	return lerp(luma, base, base);
}

float3 GetReinhard(const in float3 color, const in float exposure, const in float offset, const in float maxWhite)
{
	const float3 numerator = color * exposure * (1.0f + color / (maxWhite * maxWhite));
	return numerator / (offset + color / exposure);
}

float3 GetReinhardLuma(const in float3 color, const in float exposure, const in float offset, const in float maxWhite)
{
	const float luma = GetLuma(color);
	const float toneMappedLuma = (1.0f + luma * exposure / (maxWhite * maxWhite)) / (offset + luma / exposure);
	return color * toneMappedLuma;
}

float3 GetTonemapExposure(const in float3 color, const in float exposure)
{
	return 1.0f - exp(color * -exposure);
}

// RomBinDaHouse
float3 GetTonemapRBDH(const in float3 color, const in float exposure)
{
	return exp(-1.0f / (2.72f * exposure * color + 0.15f));
}

// Code based on ACES fitted by Stephen Hill (@self_shadow)
float3 GetACES(const in float3 color, const in float exposure)
{
	// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
	static const float3x3 ACES_INPUT_MAT =
	{
		{ 0.59719f, 0.35458f, 0.04823f },
		{ 0.07600f, 0.90834f, 0.01566f },
		{ 0.02840f, 0.13383f, 0.83777f }
	};
	// ODT_SAT => XYZ => D60_2_D65 => sRGB
	static const float3x3 ACES_OUTPUT_MAT =
	{
		{ 1.60475f, -0.53108f, -0.07367f },
		{ -0.10208f, 1.10813f, -0.00605f },
		{ -0.00327f, -0.07276f, 1.07602f }
	};
	
	const float3 mapped = mul(ACES_INPUT_MAT, color * exposure);
	
	const float3 a = mapped * (mapped + 0.0245786f) - 0.000090537f;
	const float3 b = mapped * (0.983729f * mapped + 0.4329510f) + 0.238081f;
	
	return saturate(mul(ACES_OUTPUT_MAT, a / b));
}

// ACES approximation by Krzysztof Narkowicz
float3 GetACESNautilus(const in float3 color, const in float exposure)
{
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	static const float A = 2.51f;
	static const float B = 0.03f;
	static const float C = 2.43f;
	static const float D = 0.59f;
	static const float E = 0.14f;
	
	const float3 mapped = color * 0.6f * exposure;
	return saturate((mapped * (A * mapped + B)) / (mapped * (C * mapped + D) + E));
}

// Only for LDR Rec.709 space
float3 GetKhronosPBRNeutral(const in float3 color, const in float exposure)
{
	static const float START_COMPRESSION = 0.8f - 0.04f;
	static const float DESATURATION = 0.15f;
	static const float D = 1.0f - START_COMPRESSION;
	static const float D_2 = D * D;
	
	float3 mapped = color * exposure;
	const float x = min(mapped.r, min(mapped.g, mapped.b));
	const float offset = x < 0.08f ? x - 6.25f * x * x : 0.04f;
	mapped -= offset;

	const float peak = max(mapped.r, max(mapped.g, mapped.b));
	if (peak < START_COMPRESSION)
		return mapped;

	const float newPeak = 1.0f - D_2 / (peak + D - START_COMPRESSION);
	mapped *= newPeak / peak;

	const float g = 1.0f - 1.0f / (DESATURATION * (peak - newPeak) + 1.0f);
	return lerp(mapped, newPeak, g);
}

float3 GetFilmicHable(const in float3 color, const in float exposure)
{
	// Based on Uncharted 2 tone mapping curve by John Hable
	static const float A = 0.15f; // Shoulder strength
	static const float B = 0.50f; // Linear strength
	static const float C = 0.10f; // Linear angle
	static const float D = 0.20f; // Toe strength
	static const float E = 0.02f; // Toe numerator
	static const float F = 0.30f; // Toe denominator
	static const float W = 11.2f; // Linear white point
	
	float4 packed = float4(color * exposure, W);
	packed = ((packed * (A * packed + C * B) + D * E) / (packed * (A * packed + B) + D * F)) - E / F;
	
	return packed.rgb / packed.a;
}

float3 GetFilmicVDR(const in float3 color, const in float exposure, const in float contrast, const float b, const in float c, const in float shoulder)
{
	// Introduced by Timothy Lottes at GDC 2016 for AMD's Variable Dynamic Range
	const float3 z = pow(color * exposure, contrast);
	return z / (b * pow(z, shoulder) + c);
}

float3 GetAgX(const in float3 color, const in float exposure, const in float saturation, const in float contrast, const in float midContrast)
{
	static const float3x3 AGX_INPUT_MAT =
	{
		{ 0.8566271533159880, 0.1373185106779920, 0.1118982129999500 },
		{ 0.0951212405381588, 0.7612419900249430, 0.0767997842235547 },
		{ 0.0482516061458523, 0.1014394992970650, 0.8113020027764950 }
	};
	static const float3x3 AGX_OUTPUT_MAT =
	{
		{ 1.1271467782272380, -0.1468813165635330, -0.1255038609319300 },
		{ -0.0496500000000000, 1.1084784877776500, -0.0524964871144260 },
		{ -0.0774967775043101, -0.1468813165635330, 1.2244001486462500 }
	};
	static const float MIN_EV = -10.0f;
	static const float MAX_EV = 6.5f;

	float3 agxColor = mul(AGX_INPUT_MAT, color * exposure);
	agxColor = max(agxColor, 1e-10f); // prevent log(0)
	agxColor = log2(agxColor);
	agxColor = (agxColor - MIN_EV) / (MAX_EV - MIN_EV);
	agxColor = saturate(agxColor);

	// s-curve approximation
	const float3 x = agxColor;
	const float3 x2 = x * x;
	const float3 x4 = x2 * x2;
	agxColor = 15.5f * x4 * x2 - 40.14f * x4 * x + 31.96f * x4 - 6.868f * x2 * x + 0.4298f * x2 + 0.1191f * x - 0.00232f;

	// AgX look transform - restores saturation that log encoding removes
	// without this, the image appears washed out (blender applies this by default)	
	const float luma = GetLuma(agxColor);
	const float3 offset = agxColor - luma;
	agxColor = luma + offset * saturation;
	agxColor = midContrast + (agxColor - midContrast) * contrast;
	agxColor = saturate(agxColor);
	
	return saturate(mul(AGX_OUTPUT_MAT, agxColor));
}

#endif // TONEMAP_UTILS_HLSLI