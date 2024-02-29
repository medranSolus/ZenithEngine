#ifndef SHADOW_PS_HLSLI
#define SHADOW_PS_HLSLI
#include "Buffers.hlsli"
#include "PhongFlags.hlsli"

struct ShadowBuffer
{
	float3 LightPos;
	float ParallaxScale;
	uint Flags;
};

CONSTANT(shadow, ShadowBuffer, 0);

#endif // SHADOW_PS_HLSLI