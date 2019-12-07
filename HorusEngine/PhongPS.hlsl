cbuffer LightConstantBuffer
{
	float3 lightPos;
}

static const float3 materialColor = { 0.45f, 0.6f, 0.7f };
static const float3 ambientColor = { 0.15f, 0.02f, 0.1f };
static const float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
static const float diffuseIntensity = 1.0f;

float4 main(float3 worldPos : Position, float3 normal : Normal) : SV_Target
{
	// Vertex to light data
	const float3 vertexToLight = lightPos - worldPos;
	const float distanceToLight = length(vertexToLight);
	const float3 directionToLight = vertexToLight / distanceToLight;
    
	// Diffuse attenuation
    // http://wiki.ogre3d.org/-Point+Light+Attenuation
    float attenuation = 1.0f + 0.045f * distanceToLight + 0.0075f * (distanceToLight * distanceToLight);
	
	// Diffuse intensity
    const float3 diffuse = diffuseColor * max(0.0f, dot(directionToLight, normal)) * diffuseIntensity / attenuation;
	return float4(saturate(diffuse + ambientColor), 1.0f);
}