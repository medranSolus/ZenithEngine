#ifndef CONSTANTS_FSR2_CS_HLSLI
#define CONSTANTS_FSR2_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"
#define _FSR2
#include "Utils/FfxSamplers.hlsli"

// To correctly use this cbuffer, define 'ZE_FSR2_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsFSR2
{
	FfxInt32x2 iRenderSize;
	FfxInt32x2 iMaxRenderSize;
	FfxInt32x2 iDisplaySize;
	FfxInt32x2 iInputColorResourceDimensions;
	FfxInt32x2 iLumaMipDimensions;
	FfxInt32 iLumaMipLevelToUse;
	FfxInt32 iFrameIndex;

	FfxFloat32x4 fDeviceToViewDepth;
	FfxFloat32x2 fJitter;
	FfxFloat32x2 fMotionVectorScale;
	FfxFloat32x2 fDownscaleFactor;
	FfxFloat32x2 fMotionVectorJitterCancellation;
	FfxFloat32 fPreExposure;
	FfxFloat32 fPreviousFramePreExposure;
	FfxFloat32 fTanHalfFOV;
	FfxFloat32 fJitterSequenceLength;
	FfxFloat32 fDeltaTime;
	FfxFloat32 fDynamicResChangeFactor;
	FfxFloat32 fViewSpaceToMetersFactor;
};

CBUFFER(fsr2Consts, ConstantsFSR2, 0, ZE_FSR2_CB_RANGE);

FfxInt32x2 RenderSize()
{
	return cb_fsr2Consts.iRenderSize;
}
FfxInt32x2 MaxRenderSize()
{
	return cb_fsr2Consts.iMaxRenderSize;
}
FfxInt32x2 DisplaySize()
{
	return cb_fsr2Consts.iDisplaySize;
}
FfxInt32x2 InputColorResourceDimensions()
{
	return cb_fsr2Consts.iInputColorResourceDimensions;
}
FfxInt32x2 LumaMipDimensions()
{
	return cb_fsr2Consts.iLumaMipDimensions;
}
FfxInt32 LumaMipLevelToUse()
{
	return cb_fsr2Consts.iLumaMipLevelToUse;
}
FfxInt32 FrameIndex()
{
	return cb_fsr2Consts.iFrameIndex;
}
FfxFloat32x2 Jitter()
{
	return cb_fsr2Consts.fJitter;
}
FfxFloat32x4 DeviceToViewSpaceTransformFactors()
{
	return cb_fsr2Consts.fDeviceToViewDepth;
}
FfxFloat32x2 MotionVectorScale()
{
	return cb_fsr2Consts.fMotionVectorScale;
}
FfxFloat32x2 DownscaleFactor()
{
	return cb_fsr2Consts.fDownscaleFactor;
}
FfxFloat32x2 MotionVectorJitterCancellation()
{
	return cb_fsr2Consts.fMotionVectorJitterCancellation;
}
FfxFloat32 PreExposure()
{
	return cb_fsr2Consts.fPreExposure;
}
FfxFloat32 PreviousFramePreExposure()
{
	return cb_fsr2Consts.fPreviousFramePreExposure;
}
FfxFloat32 TanHalfFoV()
{
	return cb_fsr2Consts.fTanHalfFOV;
}
FfxFloat32 JitterSequenceLength()
{
	return cb_fsr2Consts.fJitterSequenceLength;
}
FfxFloat32 DeltaTime()
{
	return cb_fsr2Consts.fDeltaTime;
}
FfxFloat32 DynamicResChangeFactor()
{
	return cb_fsr2Consts.fDynamicResChangeFactor;
}
FfxFloat32 ViewSpaceToMetersFactor()
{
	return cb_fsr2Consts.fViewSpaceToMetersFactor;
}

#include "fsr2/ffx_fsr2_resources.h"
#include "fsr2/ffx_fsr2_common.h"

#endif // CONSTANTS_FSR2_CS_HLSLI