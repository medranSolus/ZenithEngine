cbuffer SpotLightBuffer : register(b5)
{
	float3 cb_lightColor;
	float cb_lightIntensity;
	float3 cb_shadowColor;
	float cb_atteuationLinear;
	float3 cb_lightPos;
	float cb_attenuationQuad;
	float cb_innerAngle;
	float cb_outerAngle;
	float cb_falloff;
}