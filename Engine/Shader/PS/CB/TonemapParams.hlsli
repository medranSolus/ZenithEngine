#ifndef TONEMAP_PARAMS_PS_HLSLI
#define TONEMAP_PARAMS_PS_HLSLI
#include "Buffers.hlsli"

struct TonemapParams
{
	float Exposure;
};

CONSTANT(params, TonemapParams, 0);

#endif // TONEMAP_PARAMS_PS_HLSLI