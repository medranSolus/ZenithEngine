#ifndef EXPOSURE_PS_HLSLI
#define EXPOSURE_PS_HLSLI
#include "Buffers.hlsli"

struct Exposure
{
	float Val;
};

CONSTANT(exposure, Exposure, 0);

#endif // EXPOSURE_PS_HLSLI