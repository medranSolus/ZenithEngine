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

CBUFFER(light, SpotLightBuffer, 2);
CONSTANT(lightPos, float3, 1);

#endif // SPOT_LIGHT_PS_HLSLI