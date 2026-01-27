#ifndef TONEMAP_PARAMS_PS_HLSLI
#define TONEMAP_PARAMS_PS_HLSLI
#include "Buffers.hlsli"

struct TonemapParams
{
	float4 Val0;
	float Val1;
};

CONSTANT(params, TonemapParams, 0);

float GetExposure()
{
	return ct_params.Val0.x;
}

float GetReinhardOffset()
{
	return ct_params.Val0.y;
}

float GetMaxWhite()
{
	return ct_params.Val0.z;
}

float GetAgxSaturation()
{
	return ct_params.Val0.y;
}

float GetAgxContrast()
{
	return ct_params.Val0.z;
}

float GetAgxMidContrast()
{
	return ct_params.Val0.w;
}

float GetVdrShoulder()
{
	return ct_params.Val0.y;
}

float GetVDRParamB()
{
	return ct_params.Val0.z;
}

float GetVDRParamC()
{
	return ct_params.Val0.w;
}

float GetVdrConstrast()
{
	return ct_params.Val1;
}

#endif // TONEMAP_PARAMS_PS_HLSLI