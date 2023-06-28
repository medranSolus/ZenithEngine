#ifndef SOLID_PS_HLSLI
#define SOLID_PS_HLSLI
#include "Buffers.hlsli"

struct Solid
{
	float3 Color;
};

CONSTANT(solidColor, Solid, 0);

#endif // SOLID_PS_HLSLI