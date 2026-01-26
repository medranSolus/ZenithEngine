#ifndef GEOMETRY_UTILS_PS_HLSLI
#define GEOMETRY_UTILS_PS_HLSLI

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
	return float3x3(T, normalize(cross(T, N) * tan.w), N);
}

// Create offset texture coordinates (parallax occlussion mapping)
float2 GetParallaxMapping(const in float2 uv, const in float3 tangentViewDir,
	uniform float depthScale, uniform Texture2D depthMap, uniform SamplerState splr, uniform float mipBias)
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
	[unroll(MAX_LAYERS)]
	for (uint i = 0; i < MAX_LAYERS; ++i)
	{
		currentDepthMapValue = 1.0f - depthMap.SampleBias(splr, currentUV, mipBias).r;
		[branch]
		if (currentLayerDepth >= currentDepthMapValue)
			break;
		// Go to next layer
		currentUV -= deltaTexCoords;
		currentLayerDepth += layerDepth;
	}
	// Get depth values before and after current point and interpolate them for smooth intersection
	const float2 prevUV = currentUV + deltaTexCoords;
	const float nextDepth = currentDepthMapValue - currentLayerDepth;
	const float prevDepth = (1.0f - depthMap.SampleBias(splr, prevUV, mipBias).r) - currentLayerDepth + layerDepth;

	return lerp(currentUV, prevUV, nextDepth / (nextDepth - prevDepth));
}

float GetParallaxDepth(const in float2 uv, const in float3 tangentLightDir,
	uniform Texture2D depthMap, uniform SamplerState splr, uniform float mipBias)
{
	return sign(depthMap.SampleBias(splr, uv, mipBias).r);
}

float3 GetMappedNormal(const in float3x3 TBN, const in float2 texcoord,
	uniform Texture2D normalMap, uniform SamplerState splr, uniform float mipBias)
{
	// Sample normal to tangent space
	const float3 tangentNormal = normalMap.SampleBias(splr, texcoord, mipBias).rgb * 2.0f - 1.0f;
	// Transform from tangent into world space
	return normalize(mul(tangentNormal, TBN));
}

// Reconstruct pixel position from depth buffer
float3 GetWorldPosition(const in float2 texCoord, const in float depth, uniform matrix inverseViewProjection)
{
	const float x = texCoord.x * 2.0f - 1.0f;
	const float y = (1.0f - texCoord.y) * 2.0f - 1.0f;
	const float4 pos = mul(float4(x, y, depth, 1.0f), inverseViewProjection);
	return pos.xyz / pos.w;
}

#endif // GEOMETRY_UTILS_PS_HLSLI