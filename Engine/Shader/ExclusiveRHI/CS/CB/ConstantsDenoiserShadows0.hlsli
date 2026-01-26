#ifndef CONSTANTS_DENOISER_SHADOWS_0_CS_HLSLI
#define CONSTANTS_DENOISER_SHADOWS_0_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_DENOISER_SHADOWS
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_DENOISER_SHADOWS_0_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsDenoiserShadows0
{
	FfxInt32x2 BufferDimensions;
};

CBUFFER(denoiserShadowsConsts0, ConstantsDenoiserShadows0, 0, ZE_DENOISER_SHADOWS_0_CB_RANGE);

FfxInt32x2 BufferDimensions()
{
	return cb_denoiserShadowsConsts0.BufferDimensions;
}

#endif // CONSTANTS_DENOISER_SHADOWS_0_CS_HLSLI