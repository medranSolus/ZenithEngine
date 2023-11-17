#define ZE_FSR1_CB_RANGE 2
#include "CB/ConstantsFSR1.hlsli"
#ifdef _ZE_HALF_PRECISION
#   define FfxFloat3 FfxFloat16x3
#   define FfxFloat4 FfxFloat16x4
#else
#   define FfxFloat3 FfxFloat32x3
#   define FfxFloat4 FfxFloat32x4
#endif

UAV2D(upscaled, float4, 0, 0);
TEXTURE_EX(sceneInput, Texture2D<float4>, 0, 1);

void StoreEASUOutput(const in FfxUInt32x2 pxCoord, const in FfxFloat3 color)
{
	ua_upscaled[pxCoord] = float4(color, 1.f);
}

FfxFloat4 GatherEasuRed(const in FfxFloat32x2 pxCoord)
{
	return (FfxFloat4)tx_sceneInput.GatherRed(splr_LinearClamp, pxCoord, FfxInt32x2(0, 0));
}
FfxFloat4 GatherEasuGreen(const in FfxFloat32x2 pxCoord)
{
	return (FfxFloat4)tx_sceneInput.GatherGreen(splr_LinearClamp, pxCoord, FfxInt32x2(0, 0));
}
FfxFloat4 GatherEasuBlue(const in FfxFloat32x2 pxCoord)
{
	return (FfxFloat4)tx_sceneInput.GatherBlue(splr_LinearClamp, pxCoord, FfxInt32x2(0, 0));
}

#include "WarningGuardOn.hlsli"
#include "fsr1/ffx_fsr1_easu.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const uint3 ltid : SV_GroupThreadID, const uint3 wgid : SV_GroupID, const uint3 dtid : SV_DispatchThreadID)
{
	EASU(ltid, wgid, dtid);
}