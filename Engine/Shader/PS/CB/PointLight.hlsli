#ifndef POINT_LIGHT_PS_HLSLI
#define POINT_LIGHT_PS_HLSLI
#include "Buffers.hlsli"

struct PointLightBuffer
{
	float3 Color;
	float Intensity;
	float3 ShadowColor;
	float AttnLinear;
	float AttnQuad;
};

CBUFFER(light, PointLightBuffer, 1);
CONSTANT(lightPos, float3, 0);

#endif // POINT_LIGHT_PS_HLSLI