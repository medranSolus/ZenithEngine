cbuffer LightConstantBuffer
{
	float3 ambientColor;
	float atteuationConst;
	float3 lightColor;
	float atteuationLinear;
	float3 lightPos;
	float attenuationQuad;
	float lightIntensity;
}