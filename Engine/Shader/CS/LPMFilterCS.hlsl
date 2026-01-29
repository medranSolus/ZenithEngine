#define ZE_LPM_CB_RANGE 2
#include "CB/ConstantsLPM.hlsli"
#include "Utils.hlsli"
#if FFX_HALF
#	define FfxFloat3 FfxFloat16x3
#	define FfxFloat4 FfxFloat16x4
#else
#	define FfxFloat3 FfxFloat32x3
#	define FfxFloat4 FfxFloat32x4
#endif

UAV2D(outputColor, FfxFloat4, 0, 0);
TEXTURE_EX(inputColor, Texture2D<FfxFloat4>, 0, 1);

FfxFloat3 ApplyPQ(const in float3 linearColor)
{
	return FfxFloat3(ApplyPQ(linearColor, 1.0f));
}

void StoreOutput(const in FfxUInt32x2 pxCoord, FfxFloat4 color)
{
	ua_outputColor[pxCoord] = color;
}

FfxFloat4 LoadInput(const in FfxUInt32x2 pxCoord)
{
	return tx_inputColor[pxCoord];
}

#include "WarningGuardOn.hlsli"
#include "lpm/ffx_lpm_filter.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const uint3 gtid : SV_GroupThreadID, const uint3 gid : SV_GroupID, const uint3 dtid : SV_DispatchThreadID)
{
	LPMFilter(gtid, gid, dtid);
}