#define ZE_DENOISER_REFLECTIONS_CB_RANGE 9
#include "CB/ConstantsDenoiserReflections.hlsli"

UAV_EX(tileList, RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV2D(radiance, FfxFloat32x4, 1, 1);
UAV2D(variance, FfxFloat32, 2, 2);
TEXTURE_EX(roughness, Texture2D<FfxFloat32>, 0, 3);
TEXTURE_EX(radiance, Texture2D<FfxFloat32x4>, 1, 4);
TEXTURE_EX(variance, Texture2D<FfxFloat32>, 2, 5);
TEXTURE_EX(sampleCount, Texture2D<FfxFloat32>, 3, 6);
TEXTURE_EX(avgRadiance, Texture2D<FfxFloat32x3>, 4, 7);
TEXTURE_EX(reprojectedRadiance, Texture2D<FfxFloat32x4>, 5, 8);

FfxUInt32 GetDenoiserTile(const in FfxUInt32 gid)
{
	return ua_tileList[gid];
}

#ifdef _ZE_HALF_PRECISION
void FFX_DNSR_Reflections_StoreTemporalAccumulation(const in FfxInt32x2 coord, const in FfxFloat16x3 radiance, const in FfxFloat16 variance)
{
    ua_radiance[coord] = radiance.xyzz;
    ua_variance[coord] = variance.x;
}

FfxFloat16 FFX_DNSR_Reflections_LoadRoughness(const in FfxInt32x2 coord)
{
    FfxFloat16 rawRoughness = (FfxFloat16)tx_roughness.Load(FfxInt32x3(coord, 0));
    // Roughness is always linear in internal storage
    return rawRoughness * rawRoughness;
}

FfxFloat16x3 FFX_DNSR_Reflections_LoadRadiance(const in FfxInt32x2 coord)
{
    return (FfxFloat16x3)tx_radiance.Load(FfxInt32x3(coord, 0)).xyz;
}

FfxFloat16 FFX_DNSR_Reflections_LoadVariance(const in FfxInt32x2 coord)
{
    return (FfxFloat16)tx_variance.Load(FfxInt32x3(coord, 0)).x;
}

FfxFloat16 FFX_DNSR_Reflections_LoadNumSamples(const in FfxInt32x2 coord)
{
    return (FfxFloat16)tx_sampleCount.Load(FfxInt32x3(coord, 0)).x;
}

FfxFloat16x3 FFX_DNSR_Reflections_SampleAverageRadiance(const in FfxFloat32x2 uv)
{
    return (FfxFloat16x3)tx_avgRadiance.SampleLevel(splr_Linear, uv, 0.0f).xyz;
}

FfxFloat16x3 FFX_DNSR_Reflections_LoadRadianceReprojected(const in FfxInt32x2 coord)
{
    return (FfxFloat16x3)tx_reprojectedRadiance.Load(FfxInt32x3(coord, 0)).xyz;
}
#else
void FFX_DNSR_Reflections_StoreTemporalAccumulation(const in FfxInt32x2 coord, const in FfxFloat32x3 radiance, const in FfxFloat32 variance)
{
	ua_radiance[coord] = radiance.xyzz;
	ua_variance[coord] = variance.x;
}

FfxFloat32 FFX_DNSR_Reflections_LoadRoughness(const in FfxInt32x2 coord)
{
	FfxFloat32 rawRoughness = tx_roughness.Load(FfxInt32x3(coord, 0));
    // Roughness is always linear in internal storage
	return rawRoughness * rawRoughness;
}

FfxFloat32x3 FFX_DNSR_Reflections_LoadRadiance(const in FfxInt32x2 coord)
{
	return tx_radiance.Load(FfxInt32x3(coord, 0)).xyz;
}

FfxFloat32 FFX_DNSR_Reflections_LoadVariance(const in FfxInt32x2 coord)
{
	return tx_variance.Load(FfxInt32x3(coord, 0)).x;
}

FfxFloat32 FFX_DNSR_Reflections_LoadNumSamples(const in FfxInt32x2 coord)
{
	return tx_sampleCount.Load(FfxInt32x3(coord, 0)).x;
}

FfxFloat32x3 FFX_DNSR_Reflections_SampleAverageRadiance(const in FfxFloat32x2 uv)
{
	return tx_avgRadiance.SampleLevel(splr_Linear, uv, 0.0f).xyz;
}

FfxFloat32x3 FFX_DNSR_Reflections_LoadRadianceReprojected(const in FfxInt32x2 coord)
{
	return tx_reprojectedRadiance.Load(FfxInt32x3(coord, 0)).xyz;
}
#endif

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_reflections_resolve_temporal.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint gid : SV_GroupID, const in uint2 gtid : SV_GroupThreadID)
{
	ResolveTemporal(groupIndex, gid, gtid);
}