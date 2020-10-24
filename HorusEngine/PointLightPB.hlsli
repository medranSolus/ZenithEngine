cbuffer PointLightBuffer : register(b0)
{
	float3 cb_lightColor;
	float cb_lightIntensity;
	float3 cb_shadowColor;
	float cb_atteuationLinear;
	float3 cb_lightPos;
	float cb_attenuationQuad;
}