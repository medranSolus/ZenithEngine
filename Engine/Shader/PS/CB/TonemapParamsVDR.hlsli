#ifndef TONEMAP_PARAMS_VDR_PS_HLSLI
#define TONEMAP_PARAMS_VDR_PS_HLSLI
#include "Buffers.hlsli"

struct TonemapParams
{
	float Exposure;
	float Contrast;
	float B;
	float C;
	float Shoulder;
};

CONSTANT(params, TonemapParams, 0);

#endif // TONEMAP_PARAMS_VDR_PS_HLSLI