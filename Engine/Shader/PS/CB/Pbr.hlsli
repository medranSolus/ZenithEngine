#ifndef PBR_PS_HLSLI
#define PBR_PS_HLSLI
#include "Buffers.hlsli"
#include "PbrFlags.hlsli"

struct PbrBuffer
{
	float4 Albedo;
	float Metalness;
	float Roughness;
	float ParallaxScale;
	uint Flags;
};

CBUFFER(material, PbrBuffer, 0, 4);

#endif // PBR_PS_HLSLI