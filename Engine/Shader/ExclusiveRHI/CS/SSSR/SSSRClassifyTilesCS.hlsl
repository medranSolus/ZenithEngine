#define ZE_SSSR_CB_RANGE 3
#include "CB/ConstantsSSSR.hlsli"

UAV2D(blueNoise, FfxFloat32x2, 0, 0);
TEXTURE_EX(sobol, Texture2D<FfxUInt32>, 0, 1);
TEXTURE_EX(scramblingTiles, Texture2D<FfxUInt32>, 0, 2);

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
#include "sssr/ffx_sssr_classify_tiles.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const in uint localGroupIndex : SV_GroupIndex, const in uint3 gid : SV_GroupID)
{
	FfxUInt32x2 gtid = FFX_DNSR_Reflections_RemapLane8x8(localGroupIndex);
	FfxUInt32x2 dtid = WorkGroupId.xy * 8 + gtid;
	FfxFloat32 roughness = LoadRoughnessFromMaterialParametersInput(FfxInt32x3(dtid, 0));

	ClassifyTiles(dtid, gtid, roughness);

    // Extract only the channel containing the roughness to avoid loading all 4 channels in the follow up passes.
	StoreExtractedRoughness(dtid, roughness);
}