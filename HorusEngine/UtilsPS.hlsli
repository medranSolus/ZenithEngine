struct LightVectorData
{
	float3 vertexToLight;
	float distanceToLight;
	float3 directionToLight;
};

LightVectorData GetLightVectorData(uniform float3 lightPos, const in float3 vertexPos)
{
	LightVectorData lvd;
	lvd.vertexToLight = lightPos - vertexPos;
	lvd.distanceToLight = length(lvd.vertexToLight);
	lvd.directionToLight = lvd.vertexToLight / lvd.distanceToLight;
	return lvd;
}

float3 GetMappedNormal(const in float3 tan, const in float3 bitan, const in float3 normal, const in float2 texcoord,
	uniform Texture2D normalMap, uniform SamplerState splr)
{
	// Get rotation from tangent space
	const float3x3 tangentToView = float3x3
		(
			normalize(tan),
			normalize(bitan),
			normalize(normal)
		);
	// Sample normal to tangent space
	const float3 tangentNormal = normalMap.Sample(splr, texcoord).rgb * 2.0f - 1.0f;
	// Transform from tangent into view space
	return normalize(mul(tangentNormal, tangentToView));
}

float GetAttenuation(uniform float attConst, uniform float attLinear, uniform float attQuad, const in float distanceToLight)
{
	// Diffuse attenuation
	// http://wiki.ogre3d.org/-Point+Light+Attenuation
	return 1.0f / (attConst + (attLinear + attQuad * distanceToLight) * distanceToLight);
}

float3 GetDiffuse(const in float3 diffuseColor, const in float3 directionToLight, const in float3 normal, const in float attenutaion)
{
	return diffuseColor * (attenutaion * max(0.0f, dot(directionToLight, normal)));
}

float3 GetSpecular(const in float3 vertexToLight, const in float3 pos, const in float3 normal, const in float attenuation,
	const in float3 specularColor, const in float specularPower, uniform float specularIntensity)
{
	// Specular intensity based on angle between viewing vector and reflection vector
	const float3 reflection = normalize(normal * dot(vertexToLight, normal) * 2.0f - vertexToLight);
	return specularColor * (attenuation * specularIntensity * pow(max(0.0f, dot(-reflection, normalize(pos))), specularPower));
}

float GetSampledSpecularPower(const in float4 specularData)
{
	// https://gamedev.stackexchange.com/questions/74879/specular-map-what-about-the-specular-reflections-highlight-size
	return pow(2.0f, specularData.a * 13.0f);
}

float GetShadowLevel(const in float4 shadowPos, uniform SamplerState shadowSplr, uniform Texture2D shadowMap)
{
	const float3 shadowUVZ = shadowPos.xyz / shadowPos.w; // Perspecitve divide
	float level = 0.0f;
	if (shadowUVZ.z > 1.0f)
		level = 1.0f;
	else
	{
		const float zBias = shadowUVZ.z - 0.00001f;
		uint width, height;
		shadowMap.GetDimensions(width, height);
		const float dxNear = 0.5f / width;
		const float dyNear = 0.5f / height;
		const float dxFar = 1.5f / width;
		const float dyFar = 1.5f / height;

		// PCF anti-aliasing https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxNear, -dyNear)).x >= zBias ? 3.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxNear, dyFar)).x >= zBias ? 3.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxFar, dyFar)).x >= zBias ? 3.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxFar, -dyNear)).x >= zBias ? 3.0f : 0.0f;

		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxNear, dyNear)).x >= zBias ? 2.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxNear, -dyFar)).x >= zBias ? 2.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxFar, -dyFar)).x >= zBias ? 2.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxFar, dyNear)).x >= zBias ? 2.0f : 0.0f;

		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxNear, dyNear)).x >= zBias ? 1.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxFar, dyNear)).x >= zBias ? 1.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(dxFar, -dyFar)).x >= zBias ? 1.0f : 0.0f;
		level += shadowMap.Sample(shadowSplr, shadowUVZ.xy + float2(-dxNear, -dyFar)).x >= zBias ? 1.0f : 0.0f;

		level /= 24.0f;
	}
	return level;
}