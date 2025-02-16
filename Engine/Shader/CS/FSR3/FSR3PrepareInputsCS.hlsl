#define ZE_FSR3_CB_RANGE 8
#include "CB/ConstantsFSR3.hlsli"
#include "GBufferUtils.hlsli"

UAV2D(farthestDepth, FfxFloat32, 0, 0);
UAV2D(currentLuma, FfxFloat32, 1, 1);
UAV2D(reconstructPrevNearestDepth, FfxFloat32, 2, 2);
UAV2D(dilatedDepth, FfxFloat32, 3, 3);
UAV2D(dilatedMotionVectors, FfxFloat32x2, 4, 4);
TEXTURE_EX(depth, Texture2D<float>, 0, 5); // External resource format
TEXTURE_EX(colorJittered, Texture2D<AlbedoGB>, 1, 6); // External resource format
TEXTURE_EX(motionVectors, Texture2D<MotionGB>, 2, 7); // External resource format

void StoreFarthestDepth(const in FfxUInt32x2 pxCoord, const in FfxFloat32 depth)
{
	ua_farthestDepth[pxCoord] = depth;
}

void StoreCurrentLuma(const in FfxUInt32x2 pxCoord, const in FfxFloat32 luma)
{
	ua_currentLuma[pxCoord] = luma;
}

void StoreReconstructedDepth(const in FfxUInt32x2 pxCoord, const in FfxFloat32 depth)
{
	FfxUInt32 uDepth = asuint(depth);
	// min for standard, max for inverted depth
#if FFX_FSR3UPSCALER_OPTION_INVERTED_DEPTH
    InterlockedMax(ua_reconstructPrevNearestDepth[pxCoord], uDepth);
#else
	InterlockedMin(ua_reconstructPrevNearestDepth[pxCoord], uDepth);
#endif
}

void StoreDilatedDepth(const in FfxUInt32x2 pxCoord, const in FfxFloat32 depth)
{
	ua_dilatedDepth[pxCoord] = depth;
}

void StoreDilatedMotionVector(const in FfxUInt32x2 pxCoord, const in FfxFloat32x2 motionVector)
{
	ua_dilatedMotionVectors[pxCoord] = motionVector;
}

FfxFloat32 LoadInputDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_depth[pxCoord];
}

FfxFloat32x3 LoadInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_colorJittered[pxCoord].rgb;
}

FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxCoord)
{
	FfxFloat32x2 motionVector = tx_motionVectors[pxCoord];
	motionVector *= MotionVectorScale();

#if FFX_FSR3UPSCALER_OPTION_JITTERED_MOTION_VECTORS
    motionVector -= MotionVectorJitterCancellation();
#endif
	return motionVector;
}

#include "WarningGuardOn.hlsli"
#include "fsr3upscaler/ffx_fsr3upscaler_prepare_inputs.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	PrepareInputs(dtid);
}