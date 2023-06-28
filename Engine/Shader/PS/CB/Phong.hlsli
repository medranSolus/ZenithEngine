#ifndef PHONG_PS_HLSLI
#define PHONG_PS_HLSLI
#include "Buffers.hlsli"
#include "PhongFlags.hlsli"

struct PhongBuffer
{
	float4 Color;
	float3 Specular;
	uint Flags;
	float SpecularIntensity; // The bigger the brighter
	float SpecularPower;     // The smaller the less focused in one point
	float ParallaxScale;
};

CBUFFER(material, PhongBuffer, 0, 4);

#endif // PHONG_PS_HLSLI