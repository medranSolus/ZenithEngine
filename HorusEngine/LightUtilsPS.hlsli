struct LightVectorData
{
	float3 directionToLight;
	float distanceToLight;
};

// Reconstruct pixel position from depth buffer
float3 GetWorldPosition(const in float2 texCoord, const in float depth, uniform matrix inverseViewProjection)
{
	const float x = texCoord.x * 2.0f - 1.0f;
	const float y = (1.0f - texCoord.y) * 2.0f - 1.0f;
	const float4 pos = mul(float4(x, y, depth, 1.0f), inverseViewProjection);
	return pos.xyz / pos.w;
}

LightVectorData GetLightVectorData(uniform float3 lightPos, const in float3 vertexPos)
{
	const float3 vertexToLight = lightPos - vertexPos;
	LightVectorData lvd;
	lvd.distanceToLight = length(vertexToLight);
	lvd.directionToLight = vertexToLight / lvd.distanceToLight;
	return lvd;
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

float GetAttenuation(uniform float attConst, uniform float attLinear, uniform float attQuad, const in LightVectorData lvd)
{
	// http://wiki.ogre3d.org/-Point+Light+Attenuation
	return 1.0f / (attConst + (attLinear + attQuad * lvd.distanceToLight) * lvd.distanceToLight);
}

float3 GetDiffuse(const in float3 diffuseColor, const in LightVectorData lvd, const in float3 normal, const in float attenutaion)
{
	return diffuseColor * (attenutaion * max(0.0f, dot(lvd.directionToLight, normal)));
}

float GetSampledSpecularPower(const in float4 specularData)
{
	// https://gamedev.stackexchange.com/questions/74879/specular-map-what-about-the-specular-reflections-highlight-size
	return pow(2.0f, specularData.a * 13.0f);
}

float3 GetSpecular(uniform float3 cameraPos, const in LightVectorData lvd, const in float3 pos, const in float3 normal,
	const in float3 specularColor, const in float specularPower)
{
	// Halfway vector between directionToLight and directionToCamera
	const float3 H = normalize(normalize(cameraPos - pos) + lvd.directionToLight);
	return specularColor * pow(max(dot(normal, H), 0.0f), specularPower);
}

float GetShadowLevel(const in float3 pos, const in float3 normal, const in LightVectorData lvd,
	uniform float3 lightPos, uniform SamplerState shadowSplr, uniform TextureCube shadowMap)
{
	float size;
	shadowMap.GetDimensions(size, size);
	const float3 shadowPos = pos - lightPos;
	const float slope = dot(normal, lvd.directionToLight);
	float shadowLength = length(shadowPos) - 0.02f * sqrt(1.0f - slope * slope) / slope;
	float level = 0.0f;

	static const float3 OFFSETS[20] =
	{
		float3(2.0f, 2.0f, 2.0f),   float3(-2.0f, 2.0f, 2.0f),   float3(2.0f, 2.0f, 0.0f),   float3(2.0f, 0.0f, 2.0f),   float3(0.0f, 2.0f, 2.0f),
		float3(2.0f, 2.0f, -2.0f),  float3(-2.0f, 2.0f, -2.0f),  float3(2.0f, -2.0f, 0.0f),  float3(2.0f, 0.0f, -2.0f),  float3(0.0f, 2.0f, -2.0f),
		float3(2.0f, -2.0f, 2.0f),  float3(-2.0f, -2.0f, 2.0f),  float3(-2.0f, 2.0f, 0.0f),  float3(-2.0f, 0.0f, 2.0f),  float3(0.0f, -2.0f, 2.0f),
		float3(2.0f, -2.0f, -2.0f), float3(-2.0f, -2.0f, -2.0f), float3(-2.0f, -2.0f, 0.0f), float3(-2.0f, 0.0f, -2.0f), float3(0.0f, -2.0f, -2.0f)
	};
	[unroll]
	for (int i = 0; i < 20; ++i)
		level += saturate(exp(30.0f * (shadowMap.Sample(shadowSplr, shadowPos + OFFSETS[i] / size).x - shadowLength)));

	return level / 20.0f;
}