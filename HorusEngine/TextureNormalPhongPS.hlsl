cbuffer LightConstantBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightPos;
	float diffuseIntensity;
	float atteuationConst;
	float atteuationLinear;
	float attenuationQuad;
}

cbuffer TexPhongPixelBuffer
{
	float specularIntensity;
	float specularPower;
};

Texture2D tex;
Texture2D normalMap;
SamplerState splr;

float4 main(float3 cameraPos : POSITION, float3 tan : TANGENT, float3 bitan : BITANGENT, float3 normal : NORMAL, float2 tc : TEXCOORD) : SV_Target
{
	// Get rotation from tangent space
	const float3x3 tangentToView = float3x3
	(
		normalize(tan),
		normalize(bitan),
		normalize(normal)
	);
	// Sample normal to tangent space
	const float3 normalSample = normalMap.Sample(splr, tc).bgr;
	normal = normalSample * 2.0f - 1.0f;
	normal.y *= -1.0f;
	// Transform from tangent into view space
	normal = normalize(mul(normal, tangentToView));

	// Vertex to light data
	const float3 vertexToLight = lightPos - cameraPos;
	const float distanceToLight = length(vertexToLight);
	const float3 directionToLight = vertexToLight / distanceToLight;

	// Diffuse attenuation
	// http://wiki.ogre3d.org/-Point+Light+Attenuation
	float attenuation = atteuationConst + atteuationLinear * distanceToLight + attenuationQuad * (distanceToLight * distanceToLight);

	// Diffuse intensity
	const float4 diffuse = diffuseColor * max(0.0f, dot(directionToLight, normal)) * diffuseIntensity / attenuation;

	// Specular intensity based on angle between viewing vector and reflection vector
	const float3 reflection = normal * dot(vertexToLight, normal) * 2.0f - vertexToLight;
	const float4 specular = diffuseColor * (diffuseIntensity * specularIntensity * pow(max(0.0f, dot(normalize(-reflection), normalize(cameraPos))), specularPower));

	return saturate((diffuse + ambientColor) * tex.Sample(splr, tc).bgra + specular);
}