#ifndef COMMON_UTILS_HLSLI
#define COMMON_UTILS_HLSLI

// Converts a color from linear light gamma to sRGB gamma
float3 ApplyGamma(const in float3 linearColor)
{
	const float3 cutoff = step(0.0031308f, linearColor);
	const float3 higher = 1.055f * pow(linearColor, 1.0f / 2.4f) - 0.055f;
	const float3 lower = linearColor * 12.92f;

	return lerp(lower, higher, cutoff);
}

// Converts a color from sRGB gamma to linear light gamma
float3 DeleteGamma(const in float3 srgb)
{
	const float3 cutoff = step(0.04045f, srgb);
	const float3 higher = pow((srgb + 0.055f) / 1.055f, 2.4f);
	const float3 lower = srgb / 12.92f;

	return lerp(lower, higher, cutoff);
}

// Constants for ST.2084
static const float PQ_m1 = 2610.0f / 16384.0f;
static const float PQ_m2 = 2523.0f / 32.0f;
static const float PQ_c1 = 3424.0f / 4096.0f;
static const float PQ_c2 = 2413.0f / 128.0f;
static const float PQ_c3 = 2392.0f / 128.0f;

// Apply ST2084 curve on linear color
float3 ApplyPQ(const in float3 linearColor)
{
	const float3 y = pow(abs(linearColor), PQ_m1);
	return pow((PQ_c1 + PQ_c2 * y) / (1.0f + PQ_c3 * y), PQ_m2);
}

// Convert color from ST2084 curve to linear space
float3 DeletePQ(const in float3 pq)
{
	const float3 e = pow(saturate(pq), 1.0f / PQ_m2);
	return pow(max(e - PQ_c1, 0.0f) / (PQ_c2 - PQ_c3 * e), 1.0f / PQ_m1);
}

#endif // COMMON_UTILS_HLSLI