#include "CBuffer.hlsli"

struct PointLightBuffer
{
	float3 Color;
	float Intensity;
	float3 ShadowColor;
	float AttnLinear;
	float AttnQuad;
};

CBUFFER(light, PointLightBuffer, 1);
CBUFFER(lightPos, float3, 0);