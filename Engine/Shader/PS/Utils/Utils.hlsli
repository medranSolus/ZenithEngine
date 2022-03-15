#include "CommonUtils.hlsli"

float GetSampledSpecularPower(const in float specularPower)
{
	// https://gamedev.stackexchange.com/questions/74879/specular-map-what-about-the-specular-reflections-highlight-size
	return pow(2.0f, specularPower * 13.0f);
}

// Get tangent space rotation (normalized)
float3x3 GetTangentToWorld(const in float4 tan, const in float3 normal)
{
	// Make tangent again orthogonal to normal (Gramm-Schmidt process)
	const float3 N = normalize(normal);
	float3 T = normalize(tan.xyz);
	T = normalize(T - dot(T, N) * N);
	return float3x3(T, normalize(cross(N, T)) * tan.w, N);
}

// Create offset texture coordinates (parallax occlussion mapping)
float2 GetParallaxMapping(const in float2 uv, const in float3 tangentViewDir,
	uniform float depthScale, uniform Texture2D depthMap, uniform SamplerState splr)
{
	static const uint MIN_LAYERS = 8;
	static const uint MAX_LAYERS = 32;

	// More layers at higher angles for better displacement quality
	const float layers = lerp(MAX_LAYERS, MIN_LAYERS, max(dot(float3(0.0f, 0.0f, 1.0f), tangentViewDir), 0.0f));
	const float layerDepth = 1.0f / layers;
	// Shift tex coords to next layer in pixel<->camera line
	const float2 deltaTexCoords = tangentViewDir.xy * (depthScale / layers);

	float currentDepthMapValue;
	float currentLayerDepth = 0.0f;
	float2 currentUV = uv;

	// Follow pixel<->camera line to find intersection with geometry displacement (bump)
	[unroll]
	for (uint i = 0; i < MAX_LAYERS; ++i)
	{
		currentDepthMapValue = depthMap.Sample(splr, currentUV).r;
		if (currentLayerDepth >= currentDepthMapValue)
			break;
		// Go to next layer
		currentUV -= deltaTexCoords;
		currentLayerDepth += layerDepth;
	}
	// Get depth values before and after current point and interpolate them for smooth intersection
	const float2 prevUV = currentUV + deltaTexCoords;
	const float nextDepth = currentDepthMapValue - currentLayerDepth;
	const float prevDepth = depthMap.Sample(splr, prevUV).r - currentLayerDepth + layerDepth;

	return lerp(currentUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

float GetParallaxDepth(const in float2 uv, const in float3 tangentLightDir,
	uniform Texture2D depthMap, uniform SamplerState splr)
{
	float currentDepth = depthMap.Sample(splr, uv).r;
	if (currentDepth == 0.0f)
		return 0.0f;
	return 1.0f;
}

float3 GetMappedNormal(const in float3x3 TBN, const in float2 texcoord,
	uniform Texture2D normalMap, uniform SamplerState splr)
{
	// Sample normal to tangent space
	const float3 tangentNormal = normalMap.Sample(splr, texcoord).rgb * 2.0f - 1.0f;
	// Transform from tangent into world space
	return normalize(mul(tangentNormal, TBN));
}