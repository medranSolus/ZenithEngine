#include "CBuffer.hlsli"

struct DirectionalLightBuffer
{
	float3 Color;
	float Intensity;
	float3 Shadow;
};

CBUFFER(light, DirectionalLightBuffer, 1);
CBUFFER(lightDir, float3, 0);