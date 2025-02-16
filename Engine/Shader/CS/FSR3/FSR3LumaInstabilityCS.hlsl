#define ZE_FSR3_CB_RANGE 8
#include "CB/ConstantsFSR3.hlsli"

UAV2D(lumaHistory, FfxFloat32x4, 0, 0);
UAV2D(lumaInstability, FfxFloat32, 1, 1);
TEXTURE_EX(lumaHistory, Texture2D<FfxFloat32x4>, 0, 2);
TEXTURE_EX(currentLuma, Texture2D<FfxFloat32>, 1, 3);
TEXTURE_EX(dilatedMotionVectors, Texture2D<FfxFloat32x2>, 2, 4);
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 3, 5);
TEXTURE_EX(dilatedReactiveMask, Texture2D<unorm FfxFloat32x4>, 4, 6);
TEXTURE_EX(farthestDepthMip1, Texture2D<FfxFloat32>, 5, 7);

void StoreLumaHistory(const in FfxUInt32x2 pxCoord, FfxFloat32x4 history)
{
	ua_lumaHistory[pxCoord] = history;
}

void StoreLumaInstability(const in FfxUInt32x2 pxCoord, FfxFloat32 instability)
{
	ua_lumaInstability[pxCoord] = instability;
}

FfxFloat32x4 SampleLumaHistory(const in FfxFloat32x2 uv)
{
	return tx_lumaHistory.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 SampleCurrentLuma(const in FfxFloat32x2 uv)
{
	return tx_currentLuma.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32x2 LoadDilatedMotionVector(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedMotionVectors[pxCoord];
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

FfxFloat32x4 SampleDilatedReactiveMasks(const in FfxFloat32x2 uv)
{
	return tx_dilatedReactiveMask.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxInt32x2 GetFarthestDepthMip1ResourceDimensions()
{
	FfxUInt32 width;
	FfxUInt32 height;
	tx_farthestDepthMip1.GetDimensions(width, height);

	return FfxInt32x2(width, height);
}

FfxFloat32 SampleFarthestDepthMip1(FfxFloat32x2 fUV)
{
	return tx_farthestDepthMip1.SampleLevel(splr_LinearClamp, fUV, 0);
}

#include "WarningGuardOn.hlsli"
#include "fsr3upscaler/ffx_fsr3upscaler_luma_instability.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	LumaInstability(dtid);
}