#define ZE_FSR3_CB_RANGE 12
#include "CB/ConstantsFSR3.hlsli"
#include "GBufferUtils.hlsli"

UAV2D(accumulation, FfxFloat32, 0, 0);
UAV2D(newLocks, unorm FfxFloat32, 1, 1);
UAV2D(dilatedReactiveMask, unorm FfxFloat32x4, 2, 2);
TEXTURE_EX(reactiveMask, Texture2D<ReactiveGB>, 0, 3); // External resource format
TEXTURE_EX(transparencyCompositionMask, Texture2D<TransparencyGB>, 1, 4); // External resource format
TEXTURE_EX(accumulation, Texture2D<FfxFloat32>, 2, 5);
TEXTURE_EX(shadingChange, Texture2D<FfxFloat32>, 3, 6);
TEXTURE_EX(currentLuma, Texture2D<FfxFloat32>, 4, 7);
TEXTURE_EX(reconstructedPrevNearestDepthreconstructedPrevNearestDepth, Texture2D<FfxUInt32>, 5, 8);
TEXTURE_EX(dilatedMotinoVectors, Texture2D<FfxFloat32x2>, 6, 9);
TEXTURE_EX(dilatedDepth, Texture2D<FfxFloat32>, 7, 10);
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 8, 11);

void StoreAccumulation(const in FfxUInt32x2 pxCoord, const in FfxFloat32 accumulation)
{
	ua_accumulation[pxCoord] = accumulation;
}

void StoreNewLocks(const in FfxUInt32x2 pxCoord, const in FfxFloat32 newLock)
{
	ua_newLocks[pxCoord] = newLock;
}

void StoreDilatedReactiveMasks(const in FfxUInt32x2 pxCoord, const in FfxFloat32x4 mask)
{
	ua_dilatedReactiveMask[pxCoord] = mask;
}

FfxFloat32 LoadReactiveMask(const in FfxUInt32x2 pxCoord)
{
	return tx_reactiveMask[pxCoord];
}

FfxInt32x2 GetTransparencyAndCompositionMaskResourceDimensions()
{
	FfxUInt32 width;
	FfxUInt32 height;
	tx_transparencyCompositionMask.GetDimensions(width, height);

	return FfxInt32x2(width, height);
}

FfxFloat32 SampleTransparencyAndCompositionMask(const in FfxFloat32x2 uv)
{
	return tx_transparencyCompositionMask.SampleLevel(splr_LinearClamp, uv, 0).x;
}

FfxFloat32 SampleAccumulation(const in FfxFloat32x2 uv)
{
	return x_accumulation.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 SampleShadingChange(const in FfxFloat32x2 uv)
{
	return tx_shadingChange.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 LoadCurrentLuma(const in FfxUInt32x2 pxCoord)
{
	return tx_currentLuma[pxCoord];
}

FfxFloat32 LoadReconstructedPrevDepth(const in FfxUInt32x2 pxCoord)
{
	return asfloat(tx_reconstructedPrevNearestDepth[pxCoord]);
}

FfxFloat32x2 LoadDilatedMotionVector(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedMotinoVectors[pxCoord];
}

FfxFloat32 LoadDilatedDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedDepth[pxCoord];
}

FfxFloat32 Exposure()
{
	FfxFloat32 exposure = tx_exposure[FfxUInt32x2(0, 0)].x;

#if _ZE_PLATFORM_XBOX_SCARLET
    if (exposure < 0.000030517578/** 2^-15 */)
        exposure = 1.0f;
#else
	if (exposure == 0.0f)
		exposure = 1.0f;
#endif
	return exposure;
}

#include "WarningGuardOn.hlsli"
#include "fsr3upscaler/ffx_fsr3upscaler_prepare_reactivity.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	PrepareReactivity(dtid);
}