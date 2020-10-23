cbuffer DirectionalLightBuffer : register(b6)
{
	float3 cb_lightColor;
	float cb_lightIntensity;
	float3 cb_shadowColor;
	float3 cb_direction;
}