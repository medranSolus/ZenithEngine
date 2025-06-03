#ifndef CONSTANTS_FSR3_CS_HLSLI
#define CONSTANTS_FSR3_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"
#define _ZE_FFX_FSR3
#include "Utils/FfxSamplers.hlsli"

// To correctly use this cbuffer, define 'ZE_FSR3_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsFSR3
{
	FfxInt32x2 iRenderSize;
	FfxInt32x2 iPreviousFrameRenderSize;

	FfxInt32x2 iUpscaleSize;
	FfxInt32x2 iPreviousFrameUpscaleSize;

	FfxInt32x2 iMaxRenderSize;
	FfxInt32x2 iMaxUpscaleSize;

	FfxFloat32x4 fDeviceToViewDepth;

	FfxFloat32x2 fJitter;
	FfxFloat32x2 fPreviousFrameJitter;

	FfxFloat32x2 fMotionVectorScale;
	FfxFloat32x2 fDownscaleFactor;

	FfxFloat32x2 fMotionVectorJitterCancellation;
	FfxFloat32 fTanHalfFOV;
	FfxFloat32 fJitterSequenceLength;

	FfxFloat32 fDeltaTime;
	FfxFloat32 fDeltaPreExposure;
	FfxFloat32 fViewSpaceToMetersFactor;
	FfxFloat32 fFrameIndex;

	FfxFloat32 fVelocityFactor;
	FfxFloat32 fReactivenessScale;
	FfxFloat32 fShadingChangeScale;
	FfxFloat32 fAccumulationAddedPerFrame;
	
	FfxFloat32 fMinDisocclusionAccumulation;
};

CBUFFER(fsr3Consts, ConstantsFSR3, 0, ZE_FSR3_CB_RANGE);

FfxInt32x2 RenderSize()
{
	return cb_fsr3Consts.iRenderSize;
}

FfxInt32x2 PreviousFrameRenderSize()
{
	return cb_fsr3Consts.iPreviousFrameRenderSize;
}

FfxInt32x2 MaxRenderSize()
{
	return cb_fsr3Consts.iMaxRenderSize;
}

FfxInt32x2 UpscaleSize()
{
	return cb_fsr3Consts.iUpscaleSize;
}

FfxInt32x2 PreviousFrameUpscaleSize()
{
	return cb_fsr3Consts.iPreviousFrameUpscaleSize;
}

FfxInt32x2 MaxUpscaleSize()
{
	return cb_fsr3Consts.iMaxUpscaleSize;
}

FfxFloat32x2 Jitter()
{
	return cb_fsr3Consts.fJitter;
}

FfxFloat32x2 PreviousFrameJitter()
{
	return cb_fsr3Consts.fPreviousFrameJitter;
}

FfxFloat32x4 DeviceToViewSpaceTransformFactors()
{
	return cb_fsr3Consts.fDeviceToViewDepth;
}

FfxFloat32x2 MotionVectorScale()
{
	return cb_fsr3Consts.fMotionVectorScale;
}

FfxFloat32x2 DownscaleFactor()
{
	return cb_fsr3Consts.fDownscaleFactor;
}

FfxFloat32x2 MotionVectorJitterCancellation()
{
	return cb_fsr3Consts.fMotionVectorJitterCancellation;
}

FfxFloat32 TanHalfFoV()
{
	return cb_fsr3Consts.fTanHalfFOV;
}

FfxFloat32 JitterSequenceLength()
{
	return cb_fsr3Consts.fJitterSequenceLength;
}

FfxFloat32 DeltaTime()
{
	return cb_fsr3Consts.fDeltaTime;
}

FfxFloat32 DeltaPreExposure()
{
	return cb_fsr3Consts.fDeltaPreExposure;
}

FfxFloat32 ViewSpaceToMetersFactor()
{
	return cb_fsr3Consts.fViewSpaceToMetersFactor;
}

FfxFloat32 FrameIndex()
{
	return cb_fsr3Consts.fFrameIndex;
}

FfxFloat32 VelocityFactor()
{
	return cb_fsr3Consts.fVelocityFactor;
}

FfxFloat32 ReactivenessScale()
{
	return cb_fsr3Consts.fReactivenessScale;
}

FfxFloat32 ShadingChangeScale()
{
	return cb_fsr3Consts.fShadingChangeScale;
}

FfxFloat32 AccumulationAddedPerFrame()
{
    return cb_fsr3Consts.fAccumulationAddedPerFrame;
}

FfxFloat32 MinDisocclusionAccumulation()
{
	return cb_fsr3Consts.fMinDisocclusionAccumulation;
}

#include "fsr3upscaler/ffx_fsr3upscaler_resources.h"
#include "fsr3upscaler/ffx_fsr3upscaler_common.h"

#endif // CONSTANTS_FSR3_CS_HLSLI