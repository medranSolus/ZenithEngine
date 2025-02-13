#define ZE_FSR2_CB_RANGE 12
#include "CB/ConstantsFSR2.hlsli"
#include "GBufferUtils.hlsli"

UAV2D(preparedColor, FfxFloat32x4, 0, 0);
UAV2D(dilatedReactiveMask, unorm FfxFloat32x2, 1, 1);
TEXTURE_EX(colorJittered, Texture2D<AlbedoGB>, 0, 2); // External resource format
TEXTURE_EX(motionVectors, Texture2D<MotionGB>, 1, 3); // External resource format
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 2, 4);
TEXTURE_EX(reactiveMask, Texture2D<ReactiveGB>, 3, 5); // External resource format
TEXTURE_EX(transparencyCompositionMask, Texture2D<TransparencyGB>, 4, 6); // External resource format
TEXTURE_EX(reconstructedPrevNearestDepth, Texture2D<FfxUInt32>, 5, 7);
TEXTURE_EX(dilatedMotionVectors, Texture2D<FfxFloat32x2>, 6, 8);
TEXTURE_EX(prevDilatedMotionVectors, Texture2D<FfxFloat32x2>, 7, 9);
TEXTURE_EX(dilatedDepth, Texture2D<FfxFloat32>, 8, 10);
TEXTURE_EX(depth, Texture2D<float>, 9, 11); // External resource format

void StorePreparedInputColor(const in FfxUInt32x2 pxCoord, const in FfxFloat32x4 tonemapped)
{
	ua_preparedColor[pxCoord] = tonemapped;
}

void StoreDilatedReactiveMasks(const in FfxUInt32x2 pxCoord, const in FfxFloat32x2 dilatedReactiveMask)
{
	ua_dilatedReactiveMask[pxCoord] = dilatedReactiveMask;
}

FfxFloat32x3 LoadInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_colorJittered[pxCoord].rgb;
}

FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxDilatedMotionVectorPos)
{
	MotionGB srcMotionVector = tx_motionVectors[pxDilatedMotionVectorPos].xy;
	FfxFloat32x2 uvMotionVector = srcMotionVector * MotionVectorScale();

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    uvMotionVector -= MotionVectorJitterCancellation();
#endif
	return uvMotionVector;
}

FfxFloat32 Exposure()
{
	FfxFloat32 exposure = tx_exposure[FfxUInt32x2(0, 0)].x;

	if (exposure == 0.0f)
		exposure = 1.0f;
	return exposure;
}

FfxFloat32 LoadReactiveMask(const in FfxUInt32x2 pxCoord)
{
	return tx_reactiveMask[pxCoord];
}

FfxFloat32 LoadTransparencyAndCompositionMask(const in FfxUInt32x2 pxCoord)
{
	return tx_transparencyCompositionMask[pxCoord];
}

FfxFloat32 LoadReconstructedPrevDepth(const in FfxUInt32x2 pxCoord)
{
	return asfloat(tx_reconstructedPrevNearestDepth[pxCoord]);
}

FfxFloat32x2 LoadDilatedMotionVector(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedMotionVectors[pxCoord].xy;
}

FfxFloat32x2 SamplePreviousDilatedMotionVector(const in FfxFloat32x2 uv)
{
	return tx_prevDilatedMotionVectors.SampleLevel(splr_LinearClamp, uv, 0).xy;
}

FfxFloat32 LoadDilatedDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedDepth[pxCoord];
}

FfxFloat32 LoadInputDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_depth[pxCoord];
}

#include "WarningGuardOn.hlsli"
#include "fsr2/ffx_fsr2_depth_clip.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	DepthClip(dtid);
}