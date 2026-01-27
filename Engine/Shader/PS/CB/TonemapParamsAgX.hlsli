#ifndef TONEMAP_PARAMS_AGX_PS_HLSLI
#define TONEMAP_PARAMS_AGX_PS_HLSLI
#include "Buffers.hlsli"

struct TonemapParams
{
	float Exposure;
	float Saturation;
	float Contrast;
	float MidContrast;
};

CONSTANT(params, TonemapParams, 0);

#endif // TONEMAP_PARAMS_AGX_PS_HLSLI