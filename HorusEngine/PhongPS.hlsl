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

cbuffer PhongPixelBuffer
{
	float4 materialColor;
	float specularIntensity;
	float specularPower;
};

float4 main(float3 cameraPos : POSITION, float3 normal : NORMAL) : SV_Target
{
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

	return saturate((diffuse + ambientColor + specular) * materialColor);
}