#ifndef DENOISE_CS_HLSLI
#define DENOISE_CS_HLSLI
#include "Buffers.hlsli"

struct Denoise
{
	bool IsLast;
};

CONSTANT(denoise, Denoise, 0);

#endif // DENOISE_CS_HLSLI