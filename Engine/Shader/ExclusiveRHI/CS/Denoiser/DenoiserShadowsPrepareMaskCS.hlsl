#define ZE_DENOISER_SHADOWS_0_CB_RANGE 2
#include "CB/ConstantsDenoiserShadows0.hlsli"
#include "CommonUtils.hlsli"
#define TILE_SIZE_X 8
#define TILE_SIZE_Y 4

UAV_EX(shadowMask, RWStructuredBuffer<FfxUInt32>, 0, 0);
TEXTURE_EX(hitMaskResults, Texture2D<FfxUInt32>, 0, 1);

void StoreShadowMask(const in FfxUInt32 offset, const in FfxUInt32 value)
{
    ua_shadowMask[offset] = value;
}

FfxBoolean HitsLight(const in FfxUInt32x2 did, const in FfxUInt32x2 gtid, const in FfxUInt32x2 gid)
{
	return !((1U << (gtid.y * TILE_SIZE_X + gtid.x)) & tx_hitMaskResults[gid]);
}

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_shadows_prepare.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(TILE_SIZE_X, TILE_SIZE_Y, 1)]
void main(const in uint2 gtid : SV_GroupThreadID, const in uint2 gid : SV_GroupID)
{
	FFX_DNSR_Shadows_PrepareShadowMask(gtid, gid);
}