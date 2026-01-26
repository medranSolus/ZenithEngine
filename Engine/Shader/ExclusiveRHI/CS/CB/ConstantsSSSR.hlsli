#ifndef CONSTANTS_SSSR_CS_HLSLI
#define CONSTANTS_SSSR_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_SSSR
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_SSSR_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsSSSR
{	
	FfxFloat32Mat4 ViewProjectionInverse;
	FfxFloat32Mat4 Projection;
	FfxFloat32Mat4 ProjectionInverse;
	FfxFloat32Mat4 View;
	FfxFloat32Mat4 ViewInverse;
	FfxFloat32Mat4 PrevViewProjection;
	
	FfxUInt32x2 RenderSize;
	FfxFloat32x2 RenderSizeInverse;
	
	FfxFloat32 NormalsUnpackMul;
	FfxFloat32 NormalsUnpackAdd;
	FfxUInt32 RoughnessChannel;
	FfxBoolean IsRoughnessPerceptual;
	
	FfxFloat32 FactorIBL;
	FfxFloat32 TemporalStabilityFactor;
	FfxFloat32 DepthBufferThickness;
	FfxFloat32 RoughnessThreshold;
	
	FfxFloat32 VarianceThreshold;
	FfxUInt32 FrameIndex;
	FfxUInt32 MaxTraversalIntersections;
	FfxUInt32 MinTraversalOccupancy;
	
	FfxUInt32 MostDetailedMip;
	FfxUInt32 SamplesPerQuad;
	FfxUInt32 TemporalVarianceGuidedTracingEnabled;
};

CBUFFER(sssrConsts, ConstantsSSSR, 0, ZE_SSSR_CB_RANGE);

FfxFloat32Mat4 InvViewProjection()
{
	return cb_sssrConsts.ViewProjectionInverse;
}
FfxFloat32Mat4 Projection()
{
	return cb_sssrConsts.Projection;
}
FfxFloat32Mat4 InvProjection()
{
	return cb_sssrConsts.ProjectionInverse;
}
FfxFloat32Mat4 ViewMatrix()
{
	return cb_sssrConsts.View;
}
FfxFloat32Mat4 InvView()
{
	return cb_sssrConsts.ViewInverse;
}
FfxFloat32Mat4 PrevViewProjection()
{
	return cb_sssrConsts.PrevViewProjection;
}
FfxUInt32x2 RenderSize()
{
	return cb_sssrConsts.RenderSize;
}
FfxFloat32x2 InverseRenderSize()
{
	return cb_sssrConsts.RenderSizeInverse;
}
FfxFloat32 NormalsUnpackMul()
{
	return cb_sssrConsts.NormalsUnpackMul;
}
FfxFloat32 NormalsUnpackAdd()
{
	return cb_sssrConsts.NormalsUnpackAdd;
}
FfxUInt32 RoughnessChannel()
{
	return cb_sssrConsts.RoughnessChannel;
}
FfxBoolean IsRoughnessPerceptual()
{
	return cb_sssrConsts.IsRoughnessPerceptual;
}
FfxFloat32 IBLFactor()
{
	return cb_sssrConsts.FactorIBL;
}
FfxFloat32 TemporalStabilityFactor()
{
	return cb_sssrConsts.TemporalStabilityFactor;
}
FfxFloat32 DepthBufferThickness()
{
	return cb_sssrConsts.DepthBufferThickness;
}
FfxFloat32 RoughnessThreshold()
{
	return cb_sssrConsts.RoughnessThreshold;
}
FfxFloat32 VarianceThreshold()
{
	return cb_sssrConsts.VarianceThreshold;
}
FfxUInt32 FrameIndex()
{
	return cb_sssrConsts.FrameIndex;
}
FfxUInt32 MaxTraversalIntersections()
{
	return cb_sssrConsts.MaxTraversalIntersections;
}
FfxUInt32 MinTraversalOccupancy()
{
	return cb_sssrConsts.MinTraversalOccupancy;
}
FfxUInt32 MostDetailedMip()
{
	return cb_sssrConsts.MostDetailedMip;
}
FfxUInt32 SamplesPerQuad()
{
	return cb_sssrConsts.SamplesPerQuad;
}
FfxBoolean TemporalVarianceGuidedTracingEnabled()
{
	return FfxBoolean(cb_sssrConsts.TemporalVarianceGuidedTracingEnabled);
}

#endif // CONSTANTS_SSSR_CS_HLSLI