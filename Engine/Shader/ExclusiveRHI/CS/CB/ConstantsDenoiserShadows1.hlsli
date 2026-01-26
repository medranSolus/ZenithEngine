#ifndef CONSTANTS_DENOISER_SHADOWS_1_CS_HLSLI
#define CONSTANTS_DENOISER_SHADOWS_1_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_DENOISER_SHADOWS
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_DENOISER_SHADOWS_1_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsDenoiserShadows1
{
	FfxFloat32x3 Eye;
	FfxInt32 FirstFrame;
	
	FfxInt32x2 BufferDimensions;
	FfxFloat32x2 BufferDimensionsInverse;
	
	FfxFloat32x2 MotionVectorScale;
	FfxFloat32x2 NormalsUnpackMulUnpackAdd;
	
	FfxFloat32Mat4 ProjectionInverse;
	FfxFloat32Mat4 ReprojectionMatrix;
	FfxFloat32Mat4 ViewProjectionInverse;
};

CBUFFER(denoiserShadowsConsts1, ConstantsDenoiserShadows1, 0, ZE_DENOISER_SHADOWS_1_CB_RANGE);

FfxFloat32x3 Eye()
{
	return cb_denoiserShadowsConsts1.Eye;
}
FfxInt32 IsFirstFrame()
{
	return cb_denoiserShadowsConsts1.FirstFrame;
}
FfxInt32x2 BufferDimensions()
{
	return cb_denoiserShadowsConsts1.BufferDimensions;
}
FfxFloat32x2 InvBufferDimensions()
{
	return cb_denoiserShadowsConsts1.BufferDimensionsInverse;
}
FfxFloat32x2 MotionVectorScale()
{
	return cb_denoiserShadowsConsts1.MotionVectorScale;
}
FfxFloat32 NormalsUnpackMul()
{
	return cb_denoiserShadowsConsts1.NormalsUnpackMulUnpackAdd[0];
}
FfxFloat32 NormalsUnpackAdd()
{
	return cb_denoiserShadowsConsts1.NormalsUnpackMulUnpackAdd[1];
}
FfxFloat32Mat4 ProjectionInverse()
{
	return cb_denoiserShadowsConsts1.ProjectionInverse;
}
FfxFloat32Mat4 ReprojectionMatrix()
{
	return cb_denoiserShadowsConsts1.ReprojectionMatrix;
}
FfxFloat32Mat4 ViewProjectionInverse()
{
	return cb_denoiserShadowsConsts1.ViewProjectionInverse;
}

#endif // CONSTANTS_DENOISER_SHADOWS_1_CS_HLSLI