#define ZE_FSR1_CB_RANGE 2
#include "CB/ConstantsFSR1.hlsli"
#if FFX_HALF
#   define FfxInt2 FfxInt16x2
#   define FfxFloat2 FfxFloat16x2
#   define FfxFloat3 FfxFloat16x3
#   define FfxFloat4 FfxFloat16x4
#else
#   define FfxInt2 FfxInt32x2
#   define FfxFloat2 FfxFloat32x2
#   define FfxFloat3 FfxFloat32x3
#   define FfxFloat4 FfxFloat32x4
#endif

UAV2D(upscaled, float4, 0, 0);
TEXTURE_EX(rcasInput, Texture2D<float4>, 0, 1);

void StoreRCasOutputHx2(FfxInt2 pxCoord, const in FfxFloat2 fColorR, const in FfxFloat2 fColorG, const in FfxFloat2 fColorB, const in FfxFloat2 fColorA)
{
	ua_upscaled[pxCoord] = FfxFloat32x4(fColorR.x, fColorG.x, fColorB.x, fColorA.x);
	pxCoord.x += 8;
	ua_upscaled[pxCoord] = FfxFloat32x4(fColorR.y, fColorG.y, fColorB.y, fColorA.y);
}

void StoreRCasOutput(const in FfxInt2 pxCoord, const in FfxFloat3 color)
{
	ua_upscaled[pxCoord] = FfxFloat32x4(color, 1.f);
}

FfxFloat4 LoadRCas_Input(const in FfxInt2 pxCoord)
{
	return (FfxFloat4)tx_rcasInput[pxCoord];
}

#include "WarningGuardOn.hlsli"
#include "fsr1/ffx_fsr1_rcas.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const uint3 ltid : SV_GroupThreadID, const uint3 wgid : SV_GroupID, const uint3 dtid : SV_DispatchThreadID)
{
	RCAS(ltid, wgid, dtid);
}