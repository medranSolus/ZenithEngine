#ifndef CONSTANTS_DENOISER_SHADOWS_2_CS_HLSLI
#define CONSTANTS_DENOISER_SHADOWS_2_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"
#define _ZE_FFX_DENOISER_SHADOWS
#include "Utils/FfxSamplers.hlsli"

// To correctly use this cbuffer, define 'ZE_DENOISER_SHADOWS_2_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsDenoiserShadows2
{
	FfxFloat32Mat4 ProjectionInverse;
	
	FfxFloat32x2 BufferDimensionsInverse;
	FfxFloat32x2 NormalsUnpackMulUnpackAdd;
	
	FfxInt32x2 BufferDimensions;
	FfxFloat32 DepthSimilaritySigma;
};

CBUFFER(denoiserShadowsConsts2, ConstantsDenoiserShadows2, 0, ZE_DENOISER_SHADOWS_2_CB_RANGE);

FfxFloat32Mat4 ProjectionInverse()
{
	return cb_denoiserShadowsConsts2.ProjectionInverse;
}
FfxFloat32x2 InvBufferDimensions()
{
	return cb_denoiserShadowsConsts2.BufferDimensionsInverse;
}
FfxFloat32 NormalsUnpackMul()
{
	return cb_denoiserShadowsConsts2.NormalsUnpackMulUnpackAdd[0];
}
FfxFloat32 NormalsUnpackAdd()
{
	return cb_denoiserShadowsConsts2.NormalsUnpackMulUnpackAdd[1];
}
FfxInt32x2 BufferDimensions()
{
	return cb_denoiserShadowsConsts2.BufferDimensions;
}
FfxFloat32 DepthSimilaritySigma()
{
	return cb_denoiserShadowsConsts2.DepthSimilaritySigma;
}

#endif // CONSTANTS_DENOISER_SHADOWS_2_CS_HLSLI