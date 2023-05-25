#ifndef LIGHT_UTILS_PS_HLSLI
#define LIGHT_UTILS_PS_HLSLI

float GetAttenuation(uniform float attLinear, uniform float attQuad, const in float distanceToLight)
{
	// http://wiki.ogre3d.org/-Point+Light+Attenuation
	return 1.0f + (attLinear + attQuad * distanceToLight) * distanceToLight;
}

float3 GetDiffuse(const in float3 diffuseColor, const in float3 directionToLight, const in float3 normal)
{
	return diffuseColor * max(0.0f, dot(directionToLight, normal));
}

float3 GetSpecular(uniform float3 cameraPos, const in float3 directionToLight, const in float3 pos,
	const in float3 normal, const in float3 specularColor, const in float specularPower)
{
	// Halfway vector between directionToLight and directionToCamera
	const float3 H = normalize(normalize(cameraPos - pos) + directionToLight);
	return specularColor * pow(max(dot(normal, H), 0.0f), specularPower);
}

float2 GetShadowUV(const in float3 position, uniform matrix shadowViewProjection)
{
	const float4 shadowSpacePos = mul(float4(position, 1.0f), shadowViewProjection);
	return (shadowSpacePos.xy * float2(0.5f, -0.5f)) / shadowSpacePos.w + float2(0.5f, 0.5f);
}

float GetShadowLevel(const in float3 directionToCamera, const in float distanceToLight, const in float3 directionToLight,
	const in float2 shadowPos, uniform SamplerState shadowSplr, uniform Texture2D shadowMap, uniform float mapSize)
{
	const float slope = clamp(dot(directionToLight, directionToCamera), 0.0f, 0.99f);
	const float shadowLength = distanceToLight - 0.038f * sqrt(1.0f - slope * slope) / (1.0f - slope);

	float level = 0.0f;
	const float near = 0.5f / mapSize;
	const float far = 1.5f / mapSize;

	// PCF anti-aliasing https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-near, -near)).x))) * 3.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-near, far)).x))) * 3.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(far, far)).x))) * 3.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(far, -near)).x))) * 3.0f;

	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(near, near)).x))) * 2.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(near, -far)).x))) * 2.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-far, -far)).x))) * 2.0f;
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-far, near)).x))) * 2.0f;

	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-near, near)).x)));
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(far, near)).x)));
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(far, -far)).x)));
	level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + float2(-near, -far)).x)));

	return level / 24.0f;
}

float GetShadowLevel(const in float3 directionToCamera, const in float distanceToLight, const in float3 directionToLight,
	uniform SamplerState shadowSplr, uniform TextureCube shadowMap, uniform float mapSize)
{
	static const uint SAMPLES = 20;

	// NOTE: Maybe use cascaded shadow maps, maybe it will stop z fighting, to be check
	const float slope = clamp(dot(directionToLight, directionToCamera), 0.0f, 0.99f);
	const float shadowLength = distanceToLight - 0.038f * sqrt(1.0f - slope * slope) / (1.0f - slope);
	const float3 shadowPos = -1.8f * directionToLight;
	float level = 0.0f;

	// NOTE: Maybe change to some random sphere with better distribution of sample points
	static const float DIST = 1.0f;
	static const float3 OFFSETS[SAMPLES] =
	{
		float3(DIST, DIST, DIST),   float3(-DIST, DIST, DIST),   float3(DIST, DIST, 0.0f),   float3(DIST, 0.0f, DIST),   float3(0.0f, DIST, DIST),
		float3(DIST, DIST, -DIST),  float3(-DIST, DIST, -DIST),  float3(DIST, -DIST, 0.0f),  float3(DIST, 0.0f, -DIST),  float3(0.0f, DIST, -DIST),
		float3(DIST, -DIST, DIST),  float3(-DIST, -DIST, DIST),  float3(-DIST, DIST, 0.0f),  float3(-DIST, 0.0f, DIST),  float3(0.0f, -DIST, DIST),
		float3(DIST, -DIST, -DIST), float3(-DIST, -DIST, -DIST), float3(-DIST, -DIST, 0.0f), float3(-DIST, 0.0f, -DIST), float3(0.0f, -DIST, -DIST)
	};
	[unroll(SAMPLES)]
	for (uint i = 0; i < SAMPLES; ++i)
		level += saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos + OFFSETS[i] / mapSize).x)));

	return (saturate(exp(-30.0f * saturate(shadowLength - shadowMap.Sample(shadowSplr, shadowPos).x))) + level) / 21.0f;
}

#endif // LIGHT_UTILS_PS_HLSLI