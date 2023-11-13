#define ZE_FSR2_CB_RANGE 3
#include "CB/ConstantsFSR2.hlsli"

UAV2D(reconstructedPrevNearestDepth, FfxUInt32, 0, 0);
UAV2D(newLocks, unorm FfxFloat32, 1, 1);
TEXTURE_EX(luma, Texture2D<FfxFloat32>, 0, 2);

void SetReconstructedDepth(const in FfxUInt32x2 pxCoord, const in FfxUInt32 value)
{
	ua_reconstructedPrevNearestDepth[pxCoord] = value;
}

void StoreNewLocks(const in FfxUInt32x2 pxCoord, const in FfxFloat32 newLock)
{
	ua_newLocks[pxCoord] = newLock;
}

FfxFloat32 LoadLockInputLuma(const in FfxUInt32x2 pxCoord)
{
	return tx_luma[pxCoord];
}

#include "WarningGuardOn.hlsli"
#include "fsr2/ffx_fsr2_lock.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	ComputeLock(dtid);
}