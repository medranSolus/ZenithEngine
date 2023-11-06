#define ZE_FSR2_CB_RANGE 8
#include "CB/ConstantsFSR2.hlsli"

UAV2D(reconstructedPrevNearestDepth, FfxUInt32, 0, 0);
UAV2D(dilatedMotionVectors, FfxFloat32x2, 1, 1);
UAV2D(dilatedDepth, FfxFloat32, 2, 2);
UAV2D(lockLuma, FfxFloat32, 3, 3);
TEXTURE_EX(colorJittered, Texture2D<float4>, 0, 4); // External resource format
TEXTURE_EX(motionVectors, Texture2D<float2>, 1, 5); // External resource format
TEXTURE_EX(depth, Texture2D<FfxFloat32>, 2, 6); // External resource format
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 3, 7);

void StoreReconstructedDepth(const in FfxUInt32x2 pxCoord, const in FfxFloat32 depth)
{
	const FfxUInt32 uDepth = asuint(depth);
	
	// Min for standard, max for inverted depth
#if FFX_FSR2_OPTION_INVERTED_DEPTH
	InterlockedMax(ua_reconstructedPrevNearestDepth[pxCoord], uDepth);
#else
	InterlockedMin(ua_reconstructedPrevNearestDepth[pxCoord], uDepth);
#endif
}

void StoreDilatedMotionVector(const in FfxUInt32x2 pxCoord, const in FfxFloat32x2 motionVector)
{
	ua_dilatedMotionVectors[pxCoord] = motionVector;
}

void StoreDilatedDepth(const in FfxUInt32x2 pxCoord, const in FfxFloat32 depth)
{
    ua_dilatedDepth[pxCoord] = depth;
}

void StoreLockInputLuma(const in FfxUInt32x2 pxCoord, const in FfxFloat32 luma)
{
	ua_lockLuma[pxCoord] = luma;
}

FfxFloat32x3 LoadInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_colorJittered[pxCoord].rgb;
}

FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxDilatedMotionVectorPos)
{
	FfxFloat32x2 srcMotionVector = tx_motionVectors[pxDilatedMotionVectorPos].xy;
	FfxFloat32x2 uvMotionVector = srcMotionVector * MotionVectorScale();

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    uvMotionVector -= MotionVectorJitterCancellation();
#endif
	return uvMotionVector;
}

FfxFloat32 LoadInputDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_depth[pxCoord];
}

FfxFloat32 Exposure()
{
	FfxFloat32 exposure = tx_exposure[FfxUInt32x2(0, 0)].x;

	if (exposure == 0.0f)
		exposure = 1.0f;
	return exposure;
}

#include "fsr2/ffx_fsr2_reconstruct_dilated_velocity_and_previous_depth.h"

FFX_PREFER_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	ReconstructAndDilate(dtid);
}