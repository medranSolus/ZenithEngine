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
	float3 tangentNormal = normalMap.Sample(splr, texcoord).bgr * 2.0f - 1.0f;
	tangentNormal.y *= -1.0f;
	// Transform from tangent into view space
	return normalize(mul(tangentNormal, tangentToView));
}

float GetAttenuation(uniform float attConst, uniform float attLinear, uniform float attQuad, const in float distanceToLight)
{
	// Diffuse attenuation
	// http://wiki.ogre3d.org/-Point+Light+Attenuation
	return attConst + attLinear * distanceToLight + attQuad * (distanceToLight * distanceToLight);
}

float3 GetDiffuse(const in float3 diffuseColor, const in float3 directionToLight, const in float3 normal)
{
	return diffuseColor * max(0.0f, dot(directionToLight, normal));
}

float3 GetSpecular(const in float3 vertexToLight, const in float3 pos, const in float3 normal,
	const in float3 specularColor, const in float specularPower, uniform float specularIntensity)
{
	// Specular intensity based on angle between viewing vector and reflection vector
	const float3 reflection = normal * dot(vertexToLight, normal) * 2.0f - vertexToLight;
	return specularColor * specularIntensity * pow(max(0.0f, dot(normalize(-reflection), normalize(pos))), specularPower);
}

float GetSampledSpecularPower(const in float4 specularData)
{
	// https://gamedev.stackexchange.com/questions/74879/specular-map-what-about-the-specular-reflections-highlight-size
	return pow(2.0f, specularData.a * 13.0f);
}