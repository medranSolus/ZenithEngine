#ifndef DIRECTIONAL_LIGHT_PS_HLSLI
#define DIRECTIONAL_LIGHT_PS_HLSLI
#include "Buffers.hlsli"

struct DirectionalLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
};

struct LightDir
{
	float3 Dir;
};

CBUFFER(light, DirectionalLightBuffer, 1, 4);
CONSTANT(lightDir, LightDir, 0);

#endif // DIRECTIONAL_LIGHT_PS_HLSLI