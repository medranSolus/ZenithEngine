cbuffer PointLightBuffer : register(b4)
{
	float3 cb_lightColor;
	float cb_lightIntensity;
	float3 cb_shadowColor;
	float cb_atteuationLinear;
	float3 cb_lightPos;
	float cb_attenuationQuad;
}