#define ZE_SSSR_CB_RANGE 3
#include "CB/ConstantsSSSR.hlsli"

UAV2D(blueNoise, FfxFloat32x2, 0, 0);
TEXTURE_EX(sobol, Texture2D<FfxUInt32>, 0, 1);
TEXTURE_EX(scramblingTiles, Texture2D<FfxUInt32>, 1, 2);

void FFX_SSSR_StoreBlueNoiseSample(const in FfxUInt32x2 coord, const in FfxFloat32x2 blueNoise)
{
	ua_blueNoise[coord] = blueNoise;
}

FfxUInt32 FFX_SSSR_GetSobolSample(const in FfxUInt32x3 coord)
{
    return tx_sobol.Load(coord);
}

FfxUInt32 FFX_SSSR_GetScramblingTile(const in FfxUInt32x3 coord)
{
    return tx_scramblingTiles.Load(coord);
}

#include "WarningGuardOn.hlsli"
#include "sssr/ffx_sssr_prepare_blue_noise_texture.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	PrepareBlueNoiseTexture(dtid);
}