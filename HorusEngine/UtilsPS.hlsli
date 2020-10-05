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

// Encode normal into modified spherical coordinates
float2 EncodeNormal(const in float3 normal)
{
	/*
		phi = atan(y/x)
		theta = z
	*/
	return float2(atan2(normal.y, normal.x) / 3.14159265f, normal.z) * 0.5f + 0.5f;
}

// Decode normal from modified spherical coordinates
float3 DecodeNormal(const in float2 packedNormal)
{
	/*
		x = sqrt(1 - z^2) * cos(phi)
		y = sqrt(1 - z^2) * sin(phi)
		z = z
	*/
	float2 codedNormal = packedNormal * 2.0f - 1.0f;
	float2 sinCosPhi;
	sincos(codedNormal.x * 3.14159265f, sinCosPhi.x, sinCosPhi.y);
	sinCosPhi *= sqrt(1.0f - codedNormal.y * codedNormal.y);

	return float3(sinCosPhi.y, sinCosPhi.x, codedNormal.y);
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

float3 GetSpecular(uniform float3 cameraPos, const in float3 directionToLight, const in float3 pos, const in float3 normal, const in float attenuation,
	const in float3 specularColor, const in float specularPower)
{
	// Halfway vector between directionToLight and directionToCamera
	const float3 H = normalize(normalize(cameraPos - pos) + directionToLight);
	return specularColor * (attenuation * pow(saturate(dot(normal, H)), specularPower));
}

float GetSampledSpecularPower(const in float4 specularData)
{
	// https://gamedev.stackexchange.com/questions/74879/specular-map-what-about-the-specular-reflections-highlight-size
	return pow(2.0f, specularData.a * 13.0f);
}

float3 GetWorldPosition(const in float2 texCoord, const in float depth, uniform matrix inverseViewProjection)
{
	const float x = texCoord.x * 2.0f - 1.0f;
	const float y = (1.0f - texCoord.y) * 2.0f - 1.0f;
	const float4 pos = mul(float4(x, y, depth, 1.0f), inverseViewProjection);
	return pos.xyz / pos.w;
}

float GetShadowLevel(const in float3 pos, const in float3 normal, const in float3 directionToLight,
	uniform float3 lightPos, uniform SamplerState shadowSplr, uniform TextureCube shadowMap)
{
	float size;
	shadowMap.GetDimensions(size, size);
	const float3 shadowPos = pos - lightPos;
	const float slope = dot(normal, directionToLight);
	float shadowLength = length(shadowPos) - 0.02f * sqrt(1.0f - slope * slope) / slope;
	float level = 0.0f;

	static const int MSAA_RANGE = 3;
	[unroll]
	for (int x = -MSAA_RANGE; x <= MSAA_RANGE; ++x)
	{
		[unroll]
		for (int y = -MSAA_RANGE; y <= MSAA_RANGE; ++y)
		{
			[unroll]
			for (int z = -MSAA_RANGE; z <= MSAA_RANGE; ++z)
				level += saturate(exp(30.0f * (shadowMap.Sample(shadowSplr, shadowPos + float3(x, y, z) / size).x - shadowLength)));
		}
	}

	return level / ((MSAA_RANGE * 2 + 1) * (MSAA_RANGE * 2 + 1) * (MSAA_RANGE * 2 + 1));
}