cbuffer LightConstantBuffer
{
	float3 lightPos;
}

static const float3 materialColor = { 0.45f, 0.6f, 0.7f };
static const float3 ambientColor = { 0.15f, 0.02f, 0.1f };
static const float3 diffuseColor = { 1.0f, 1.0f, 1.0f };
static const float diffuseIntensity = 5.0f;

float4 main(float3 worldPos : Position, float3 normal : Normal) : SV_Target
{
	// Vertex to light data
	const float3 vertexToLight = lightPos - worldPos;
	const float distanceToLight = length(vertexToLight);
	const float3 directionToLight = vertexToLight / distanceToLight;
    
	// Diffuse attenuation
    // http://wiki.ogre3d.org/-Point+Light+Attenuation
    float attenuation = 1.0f;
    [branch] if (distanceToLight <= 7.0f)
        attenuation += 0.7f * distanceToLight + 1.8f * (distanceToLight * distanceToLight);
    else if (distanceToLight < 13.0f)
        attenuation += 0.35f * distanceToLight + 0.44f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 20.0f)
        attenuation += 0.22f * distanceToLight + 0.2f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 32.0f)
        attenuation += 0.14f * distanceToLight + 0.07f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 50.0f)
        attenuation += 0.09f * distanceToLight + 0.032f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 65.0f)
        attenuation += 0.07f * distanceToLight + 0.017f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 100.0f)
        attenuation += 0.045f * distanceToLight + 0.0075f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 160.0f)
        attenuation += 0.027f * distanceToLight + 0.0028f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 200.0f)
        attenuation += 0.022f * distanceToLight + 0.0019f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 325.0f)
        attenuation += 0.014f * distanceToLight + 0.0007f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 600.0f)
        attenuation += 0.007f * distanceToLight + 0.0002f * (distanceToLight * distanceToLight);
    else if (distanceToLight <= 3250.0f)
        attenuation += 0.0014f * distanceToLight + 0.000007f * (distanceToLight * distanceToLight);
    else
        attenuation += 0.0005f * distanceToLight + 0.0000003f * (distanceToLight * distanceToLight);
	
	// Diffuse intensity
    const float3 diffuse = diffuseColor * max(0.0f, dot(directionToLight, normal)) * diffuseIntensity / attenuation;
	return float4(saturate(diffuse + ambientColor), 1.0f);
}