cbuffer LightConstantBuffer
{
    float4 materialColor;
    float4 ambientColor;
    float4 diffuseColor;
	float3 lightPos;
    float diffuseIntensity;
    float atteuationLinear;
    float attenuationQuad;
}

float4 main(float3 worldPos : Position, float3 normal : Normal) : SV_Target
{
	// Vertex to light data
	const float3 vertexToLight = lightPos - worldPos;
	const float distanceToLight = length(vertexToLight);
	const float3 directionToLight = vertexToLight / distanceToLight;
    
	// Diffuse attenuation
    // http://wiki.ogre3d.org/-Point+Light+Attenuation
    float attenuation = 1.0f + atteuationLinear * distanceToLight + attenuationQuad * (distanceToLight * distanceToLight);
	
	// Diffuse intensity
    const float4 diffuse = diffuseColor * max(0.0f, dot(directionToLight, normal)) * diffuseIntensity / attenuation;
    return saturate((diffuse + ambientColor) * materialColor);
}