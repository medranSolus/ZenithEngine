#ifndef CONSTANTS_DENOISER_REFLECTIONS_CS_HLSLI
#define CONSTANTS_DENOISER_REFLECTIONS_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"
#define _ZE_FFX_DENOISER_REFLECTIONS
#include "Utils/FfxSamplers.hlsli"

// To correctly use this cbuffer, define 'ZE_DENOISER_REFLECTIONS_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsDenoiser
{
	FfxFloat32Mat4 ProjectionInverse;
	FfxFloat32Mat4 ViewInverse;
	FfxFloat32Mat4 PreviousViewProjection;
	
	FfxUInt32x2 RenderSize;
	FfxFloat32x2 RenderSizeInverse;
	
	FfxFloat32x2 MotionVectorScale;
	FfxFloat32 NormalsUnpackMul;
	FfxFloat32 NormalsUnpackAdd;
	
	FfxBoolean IsRoughnessPerceptual;
	FfxFloat32 TemporalStabilityFactor;
	FfxFloat32 RoughnessThreshold;
};

CBUFFER(denoiserReflectionsConsts, ConstantsDenoiser, 0, ZE_DENOISER_REFLECTIONS_CB_RANGE);

FfxFloat32Mat4 InvProjection()
{
	return cb_denoiserReflectionsConsts.ProjectionInverse;
}
FfxFloat32Mat4 InvView()
{
	return cb_denoiserReflectionsConsts.ViewInverse;
}
FfxFloat32Mat4 PrevViewProjection()
{
	return cb_denoiserReflectionsConsts.PreviousViewProjection;
}
FfxUInt32x2 RenderSize()
{
	return cb_denoiserReflectionsConsts.RenderSize;
}
FfxFloat32x2 InverseRenderSize()
{
	return cb_denoiserReflectionsConsts.RenderSizeInverse;
}
FfxFloat32x2 MotionVectorScale()
{
	return cb_denoiserReflectionsConsts.MotionVectorScale;
}
FfxFloat32 NormalsUnpackMul()
{
	return cb_denoiserReflectionsConsts.NormalsUnpackMul;
}
FfxFloat32 NormalsUnpackAdd()
{
	return cb_denoiserReflectionsConsts.NormalsUnpackAdd;
}
FfxBoolean IsRoughnessPerceptual()
{
	return cb_denoiserReflectionsConsts.IsRoughnessPerceptual;
}
FfxFloat32 TemporalStabilityFactor()
{
	return cb_denoiserReflectionsConsts.TemporalStabilityFactor;
}
FfxFloat32 RoughnessThreshold()
{
	return cb_denoiserReflectionsConsts.RoughnessThreshold;
}

#endif // CONSTANTS_DENOISER_REFLECTIONS_CS_HLSLI