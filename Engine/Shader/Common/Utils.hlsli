#ifndef COMMON_UTILS_HLSLI
#define COMMON_UTILS_HLSLI

static const float PI = 3.14159265f;

// 1 in the linear frame-buffer space corresponds to this value of physical luminance (typically 100 cd/m^2) [cd/m^2]
static const float REFERENCE_LUMINANCE = 100.0f;

// Constants for ST.2084
static const float PQ_M1 = 2610.0f / 16384.0f;
static const float PQ_M2 = 2523.0f / 32.0f;
static const float PQ_C1 = 3424.0f / 4096.0f;
static const float PQ_C2 = 2413.0f / 128.0f;
static const float PQ_C3 = 2392.0f / 128.0f;
static const float PQ_PQC = 10000.0f; // Max supported luminance

// Converts linear frame-buffer value to physical luminance where 1 corresponds to REFERENCE_LUMINANCE
float3 GetLuminanceColor(float3 color)
{
	return color * REFERENCE_LUMINANCE;
}

// Converts physical luminance to a linear frame-buffer value, where 1 corresponds to REFERENCE_LUMINANCE
float3 GetColorFromLuminance(float3 luminance)
{
	return luminance / REFERENCE_LUMINANCE;
}

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

// Apply ST2084 curve on linear color (inverse EOTF), exponentScaleFactor should be normally 1
float3 ApplyPQ(const in float3 linearColor, float exponentScaleFactor)
{
	const float3 y = pow(saturate(GetLuminanceColor(linearColor) / PQ_PQC), PQ_M1);
	return exp2(exponentScaleFactor * PQ_M2 * (log2(PQ_C1 + PQ_C2 * y) - log2(1.0f + PQ_C3 * y)));
}

// Convert color from ST2084 curve to linear space (EOTF), exponentScaleFactor should be normally 1
float3 DeletePQ(const in float3 pq, float exponentScaleFactor)
{
	const float3 e = pow(saturate(pq), 1.0f / (PQ_M2 * exponentScaleFactor));
	return GetColorFromLuminance(pow(max(e - PQ_C1, 0.0f) / (PQ_C2 - PQ_C3 * e), 1.0f / PQ_M1) * PQ_PQC);
}

// Encodes a 3D unit vector into a 2D vector in [-1, 1]
float2 EncodeOctahedral(const in float3 nrm)
{
	float3 n = nrm / dot(abs(nrm), 1.0f);
	float2 folded = (1.0 - abs(n.yx)) * (step(0.0f, n.xy) * 2.0f - 1.0f);
	return lerp(folded, n.xy, step(0.0f, n.z));
}

// Decodes a 2D encoded vector back into a 3D unit vector
float3 DecodeOctahedral(const in float2 oct)
{
	float3 n = float3(oct.xy, 1.0f - abs(oct.x) - abs(oct.y));
	n.xy -= max(-n.z, 0.0) * (step(0.0f, n.xy) * 2.0f - 1.0f);
	return normalize(n);
}

#endif // COMMON_UTILS_HLSLI