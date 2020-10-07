cbuffer PointLightBuffer : register(b4)
{
	float3 cb_ambientColor;
	float cb_atteuationConst;
	float3 cb_lightColor;
	float cb_atteuationLinear;
	float3 cb_lightPos;
	float cb_attenuationQuad;
	float3 cb_shadowColor;
	float cb_lightIntensity;
}