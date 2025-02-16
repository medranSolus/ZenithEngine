#define ZE_FSR3_CB_RANGE 11
#define ZE_SPD_CB_RANGE 12
#include "CB/ConstantsFSR3.hlsli"
#include "CB/ConstantsSPD.hlsli"

UAV2D(farthestDepthMip1, FfxFloat32, 0, 0);
UAV2D(frameInfo, FfxFloat32x4, 1, 1);
UAV_EX(spdMip0, RWTexture2D<FfxFloat32x2>, 2, 2);
UAV_EX(spdMip1, RWTexture2D<FfxFloat32x2>, 3, 3);
UAV_EX(spdMip2, RWTexture2D<FfxFloat32x2>, 4, 4);
UAV_EX(spdMip3, RWTexture2D<FfxFloat32x2>, 5, 5);
UAV_EX(spdMip4, RWTexture2D<FfxFloat32x2>, 6, 6);
UAV_EX(spdMip5, globallycoherent RWTexture2D<FfxFloat32x2>, 7, 7);
UAV_EX(spdGlobalAtomic, globallycoherent RWTexture2D<FfxUInt32>, 8, 8);
TEXTURE_EX(farthestDepth, Texture2D<FfxFloat32>, 0, 9);
TEXTURE_EX(currentLuma, Texture2D<FfxFloat32>, 1, 10);

void StoreFarthestDepthMip1(FfxUInt32x2 iPxPos, FfxFloat32 fDepth)
{
	ua_farthestDepthMip1[iPxPos] = fDepth;
}

FfxFloat32x4 LoadFrameInfo()
{
	return ua_frameInfo[FfxInt32x2(0, 0)];
}

void StoreFrameInfo(const in FfxFloat32x4 info)
{
	ua_frameInfo[FfxInt32x2(0, 0)] = info;
}

FfxFloat32x2 RWLoadPyramid(const in FfxInt32x2 pxCoord, const in FfxUInt32 index)
{
	switch (index)
	{
	default:
		return 0.0f;
	case 0:
		return ua_spdMip0[pxCoord];
	case 1:
		return ua_spdMip1[pxCoord];
	case 2:
		return ua_spdMip2[pxCoord];
	case 3:
		return ua_spdMip3[pxCoord];
	case 4:
		return ua_spdMip4[pxCoord];
	case 5:
		return ua_spdMip5[pxCoord];
	}
}

void StorePyramid(const in FfxInt32x2 pxCoord, const in FfxFloat32x2 value, const in FfxUInt32 index)
{
	switch (index)
	{
	case 0:
		ua_spdMip0[pxCoord] = value;
		break;
	case 1:
		ua_spdMip1[pxCoord] = value;
		break;
	case 2:
		ua_spdMip2[pxCoord] = value;
		break;
	case 3:
		ua_spdMip3[pxCoord] = value;
		break;
	case 4:
		ua_spdMip4[pxCoord] = value;
		break;
	case 5:
		ua_spdMip5[pxCoord] = value;
		break;
	}
}

void SPD_IncreaseAtomicCounter(inout FfxUInt32 spdCounter)
{
	InterlockedAdd(ua_spdGlobalAtomic[FfxInt32x2(0, 0)], 1, spdCounter);
}

void SPD_ResetAtomicCounter()
{
	ua_spdGlobalAtomic[FfxInt32x2(0, 0)] = 0;
}

FfxFloat32 LoadFarthestDepth(const in FfxUInt32x2 pxCoord)
{
	return tx_farthestDepth[pxCoord];
}

FfxFloat32 LoadCurrentLuma(const in FfxUInt32x2 pxCoord)
{
	return tx_currentLuma[pxCoord];
}

#include "WarningGuardOn.hlsli"
#include "fsr3upscaler/ffx_fsr3upscaler_luma_pyramid.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(256, 1, 1)]
void main(const uint3 wgid : SV_GroupID, const uint tid : SV_GroupIndex)
{
	ComputeAutoExposure(wgid, tid);
}