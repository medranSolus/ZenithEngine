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

float3 GetMappedNormal(const in float3 tan, const in float3 bitan, const in float3 normal,
	const in float2 texcoord, uniform Texture2D normalMap, uniform SamplerState splr)
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

float GetShadowLevel(const in float3 shadowPos, uniform SamplerComparisonState shadowSplr, uniform TextureCube shadowMap)
{
	return shadowMap.SampleCmpLevelZero(shadowSplr, shadowPos, length(shadowPos) / 1000.0f);
	//const float3 shadowUVZ = shadowPos.xyz / shadowPos.w; // Perspecitve divide
	//float level = 0.0f;
	//if (shadowUVZ.z > 1.0f || shadowUVZ.z < 0.0f)
	//	level = 1.0f;
	//else
	//{
	//	// PCF anti-aliasing https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
	//	static const int PCF_RANGE = 2;
	//	[unroll]
	//	for (int x = -PCF_RANGE; x <= PCF_RANGE; ++x)
	//	{
	//		[unroll]
	//		for (int y = -PCF_RANGE; y <= PCF_RANGE; ++y)
	//			level += shadowMap.SampleCmpLevelZero(shadowSplr, shadowUVZ.xy, shadowUVZ.z, int2(x, y)); // Hardware comparison (hardware PCF 2x2 grid)
	//	}
	//	level /= (PCF_RANGE * 2 + 1) * (PCF_RANGE * 2 + 1);
	//}
	//return level;
}