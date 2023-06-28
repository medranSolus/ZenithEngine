#ifndef SPOT_LIGHT_PS_HLSLI
#define SPOT_LIGHT_PS_HLSLI
#include "Buffers.hlsli"

struct SpotLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
	float InnerAngle;
	float3 Direction;
	float OuterAngle;
	float AttnLinear;
	float AttnQuad;
};

struct LightPos
{
	float3 Pos;
};

CBUFFER(light, SpotLightBuffer, 2, 5);
CONSTANT(lightPos, LightPos, 1);

#endif // SPOT_LIGHT_PS_HLSLI