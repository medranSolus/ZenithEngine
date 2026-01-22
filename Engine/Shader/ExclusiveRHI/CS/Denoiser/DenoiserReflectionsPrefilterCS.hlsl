#define ZE_DENOISER_REFLECTIONS_CB_RANGE 9
#include "CB/ConstantsDenoiserReflections.hlsli"
#include "GBufferUtils.hlsli"

UAV_EX(tileList, RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV2D(radiance, FfxFloat32x4, 1, 1);
UAV2D(variance, FfxFloat32, 2, 2);
TEXTURE_EX(radiance, Texture2D<FfxFloat32x4>, 0, 3);
TEXTURE_EX(variance, Texture2D<FfxFloat32>, 1, 4);
TEXTURE_EX(normals, Texture2D<CodedNormalGB>, 2, 5); // External resource format
TEXTURE_EX(roughness, Texture2D<FfxFloat32>, 3, 6);
TEXTURE_EX(avgRadiance, Texture2D<FfxFloat32x3>, 4, 7);
TEXTURE_EX(depthHierarchy, Texture2D<FfxFloat32>, 5, 8);

FfxUInt32 GetDenoiserTile(const in FfxUInt32 gid)
{
	return ua_tileList[gid];
}

#if FFX_HALF
void FFX_DNSR_Reflections_StorePrefilteredReflections(const in FfxInt32x2 coord, const in FfxFloat16x3 radiance, const in FfxFloat16 variance)
{
    ua_radiance[coord] = radiance.xyzz;
    ua_variance[coord] = variance;
}
#else
void FFX_DNSR_Reflections_StorePrefilteredReflections(const in FfxInt32x2 coord, const in FfxFloat32x3 radiance, const in FfxFloat32 variance)
{
	ua_radiance[coord] = radiance.xyzz;
	ua_variance[coord] = variance;
}
#endif

#if FFX_HALF
FfxFloat16x3 LoadRadianceH(const in FfxInt32x3 coord)
{
    return (FfxFloat16x3)tx_radiance.Load(coord).xyz;
}

FfxFloat16 LoadVarianceH(const in FfxInt32x3 coord)
{
    return (FfxFloat16)tx_variance.Load(coord).x;
}

FfxFloat16x3 FFX_DENOISER_LoadWorldSpaceNormalH(const in FfxInt32x2 coord)
{
	return (FfxFloat16x3)DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)));
}

FfxFloat16 FFX_DNSR_Reflections_LoadRoughness(const in FfxInt32x2 coord)
{
    FfxFloat16 rawRoughness = (FfxFloat16)tx_roughness.Load(FfxInt32x3(coord, 0));
    // Roughness is always linear in internal storage
    return rawRoughness * rawRoughness;
}

FfxFloat16x3 FFX_DNSR_Reflections_SampleAverageRadiance(const in FfxFloat32x2 uv)
{
    return (FfxFloat16x3)tx_avgRadiance.SampleLevel(splr_LinearClamp, uv, 0.0f).xyz;
}
#else
FfxFloat32x3 LoadRadiance(const in FfxInt32x3 coord)
{
	return tx_radiance.Load(coord).xyz;
}

FfxFloat32 LoadVariance(const in FfxInt32x3 coord)
{
	return tx_variance.Load(coord).x;
}

FfxFloat32x3 FFX_DENOISER_LoadWorldSpaceNormal(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)));
}

FfxFloat32 FFX_DNSR_Reflections_LoadRoughness(const in FfxInt32x2 coord)
{
	FfxFloat32 rawRoughness = tx_roughness.Load(FfxInt32x3(coord, 0));
    // Roughness is always linear in internal storage
	return rawRoughness * rawRoughness;
}

FfxFloat32x3 FFX_DNSR_Reflections_SampleAverageRadiance(const in FfxFloat32x2 uv)
{
	return tx_avgRadiance.SampleLevel(splr_LinearClamp, uv, 0.0f).xyz;
}
#endif

FfxFloat32 FFX_DENOISER_LoadDepth(const in FfxInt32x2 coord, const in FfxInt32 mip)
{
	return tx_depthHierarchy.Load(FfxInt32x3(coord, mip));
}

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_reflections_prefilter.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint gid : SV_GroupID, const in uint2 gtid : SV_GroupThreadID)
{
	Prefilter(groupIndex, gid, gtid);
}