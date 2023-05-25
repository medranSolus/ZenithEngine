#ifndef DIRECTIONAL_LIGHT_PS_HLSLI
#define DIRECTIONAL_LIGHT_PS_HLSLI
#include "Buffers.hlsli"

struct DirectionalLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
};

CBUFFER(light, DirectionalLightBuffer, 1);
CONSTANT(lightDir, float3, 0);

#endif // DIRECTIONAL_LIGHT_PS_HLSLI