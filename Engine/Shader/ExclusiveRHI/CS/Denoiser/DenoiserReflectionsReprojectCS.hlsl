#define ZE_DENOISER_REFLECTIONS_CB_RANGE 16
#include "CB/ConstantsDenoiserReflections.hlsli"
#include "CommonUtils.hlsli"

UAV2D(reprojectedRadiance, FfxFloat32x3, 0, 0);
UAV2D(variance, FfxFloat32, 1, 1);
UAV2D(sampleCount, FfxFloat32, 2, 2);
UAV2D(avgRadiance, FfxFloat32x3, 3, 3);
UAV_EX(tileList, RWStructuredBuffer<FfxUInt32>, 4, 4);
TEXTURE_EX(roughness, Texture2D<FfxFloat32>, 0, 5);
TEXTURE_EX(radiance, Texture2D<FfxFloat32x4>, 1, 6);
TEXTURE_EX(variance, Texture2D<FfxFloat32>, 2, 7);
TEXTURE_EX(normals, Texture2D<float2>, 3, 8); // External resource format
TEXTURE_EX(normalsHistory, Texture2D<float2>, 4, 9); // External resource format
TEXTURE_EX(radianceHistory, Texture2D<FfxFloat32x4>, 5, 10);
TEXTURE_EX(roughnessHistory, Texture2D<FfxFloat32>, 6, 11);
TEXTURE_EX(depthHistory, Texture2D<FfxFloat32>, 7, 12);
TEXTURE_EX(depthHierarchy, Texture2D<FfxFloat32>, 8, 13);
TEXTURE_EX(motionVectors, Texture2D<float2>, 9, 14); // External resource format
TEXTURE_EX(sampleCount, Texture2D<FfxFloat32>, 10, 15);

#ifdef _ZE_HALF_PRECISION
void FFX_DNSR_Reflections_StoreRadianceReprojected(const in FfxInt32x2 coord, const in FfxFloat16x3 value)
{
	ua_reprojectedRadiance[coord] = value;
}

void FFX_DNSR_Reflections_StoreVariance(const in FfxInt32x2 coord, const in FfxFloat16 value)
{
    ua_variance[coord] = value;
}

void FFX_DNSR_Reflections_StoreNumSamples(const in FfxInt32x2 coord, const in FfxFloat16 value)
{
    ua_sampleCount[coord] = value;
}

void FFX_DNSR_Reflections_StoreAverageRadiance(const in FfxInt32x2 coord, const in FfxFloat16x3 value)
{
    ua_avgRadiance[coord] = value;
}
#else
void FFX_DNSR_Reflections_StoreRadianceReprojected(const in FfxInt32x2 coord, const in FfxFloat32x3 value)
{
	ua_reprojectedRadiance[coord] = value;
}

void FFX_DNSR_Reflections_StoreVariance(const in FfxInt32x2 coord, const in FfxFloat32 value)
{
	ua_variance[coord] = value;
}

void FFX_DNSR_Reflections_StoreNumSamples(const in FfxInt32x2 coord, const in FfxFloat32 value)
{
	ua_sampleCount[coord] = value;
}

void FFX_DNSR_Reflections_StoreAverageRadiance(const in FfxInt32x2 coord, const in FfxFloat32x3 value)
{
	ua_avgRadiance[coord] = value;
}
#endif

FfxUInt32 GetDenoiserTile(const in FfxUInt32 gid)
{
	return ua_tileList[gid];
}

#ifdef _ZE_HALF_PRECISION
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

FfxFloat16 FFX_DNSR_Reflections_LoadRayLength(const in FfxInt32x2 coord)
{
    return (FfxFloat16)tx_radiance.Load(FfxInt32x3(coord, 0)).w;
}

FfxFloat16 FFX_DNSR_Reflections_SampleVarianceHistory(const in FfxFloat32x2 uv)
{
    return (FfxFloat16)tx_variance.SampleLevel(splr_Linear, uv, 0.0f).x;
}

FfxFloat16x3 FFX_DNSR_Reflections_LoadWorldSpaceNormal(const in FfxInt32x2 coord)
{
	return (FfxFloat16x3)DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)).xy);
}

FfxFloat16x3 FFX_DNSR_Reflections_SampleWorldSpaceNormalHistory(const in FfxFloat32x2 uv)
{
    return (FfxFloat16x3)DecodeNormal(tx_normalsHistory.SampleLevel(splr_Linear, uv, 0.0f).xy);
}

FfxFloat16x3 FFX_DNSR_Reflections_LoadWorldSpaceNormalHistory(const in FfxInt32x2 coord)
{
    return (FfxFloat16x3)DecodeNormal(tx_normalsHistory.Load(FfxInt32x3(coord, 0)));
}

FfxFloat16x3 FFX_DNSR_Reflections_SampleRadianceHistory(const in FfxFloat32x2 uv)
{
    return (FfxFloat16x3)tx_radianceHistory.SampleLevel(splr_Linear, uv, 0.0f).xyz;
}

FfxFloat16x3 FFX_DNSR_Reflections_LoadRadianceHistory(const in FfxInt32x2 coord)
{
    return (FfxFloat16x3)tx_radianceHistory.Load(FfxInt32x3(coord, 0)).xyz;
}

FfxFloat16 FFX_DNSR_Reflections_SampleRoughnessHistory(const in FfxFloat32x2 uv)
{
    FfxFloat16 rawRoughness = (FfxFloat16)tx_roughnessHistory.SampleLevel(splr_Linear, uv, 0.0f);
    // Roughness is always linear in internal storage
    return rawRoughness * rawRoughness;
}

FfxFloat16 FFX_DNSR_Reflections_SampleNumSamplesHistory(const in FfxFloat32x2 uv)
{
    return (FfxFloat16)tx_sampleCount.SampleLevel(splr_Linear, uv, 0.0f).x;
}
#else
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

FfxFloat32 FFX_DNSR_Reflections_LoadRayLength(const in FfxInt32x2 coord)
{
	return tx_radiance.Load(FfxInt32x3(coord, 0)).w;
}

FfxFloat32 FFX_DNSR_Reflections_SampleVarianceHistory(const in FfxFloat32x2 uv)
{
	return tx_variance.SampleLevel(splr_Linear, uv, 0.0f).x;
}

FfxFloat32x3 FFX_DNSR_Reflections_LoadWorldSpaceNormal(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)).xy);
}

FfxFloat32x3 FFX_DNSR_Reflections_SampleWorldSpaceNormalHistory(const in FfxFloat32x2 uv)
{
	return DecodeNormal(tx_normalsHistory.SampleLevel(splr_Linear, uv, 0.0f).xy);
}

FfxFloat32x3 FFX_DNSR_Reflections_LoadWorldSpaceNormalHistory(const in FfxInt32x2 coord)
{
    return DecodeNormal(tx_normalsHistory.Load(FfxInt32x3(coord, 0)));
}

FfxFloat32x3 FFX_DNSR_Reflections_SampleRadianceHistory(const in FfxFloat32x2 uv)
{
    return (FfxFloat32x3)tx_radianceHistory.SampleLevel(splr_Linear, uv, 0.0f).xyz;
}

FfxFloat32x3 FFX_DNSR_Reflections_LoadRadianceHistory(const in FfxInt32x2 coord)
{
	return tx_radianceHistory.Load(FfxInt32x3(coord, 0)).xyz;
}

FfxFloat32 FFX_DNSR_Reflections_SampleRoughnessHistory(const in FfxFloat32x2 uv)
{
	FfxFloat32 rawRoughness = tx_roughnessHistory.SampleLevel(splr_Linear, uv, 0.0f);
    // Roughness is always linear in internal storage
	return rawRoughness * rawRoughness;
}

FfxFloat32 FFX_DNSR_Reflections_SampleNumSamplesHistory(const in FfxFloat32x2 uv)
{
	return tx_sampleCount.SampleLevel(splr_Linear, uv, 0.0f).x;
}
#endif

FfxFloat32 FFX_DNSR_Reflections_SampleDepthHistory(const in FfxFloat32x2 uv)
{
	return tx_depthHistory.SampleLevel(splr_Linear, uv, 0.0f);
}

FfxFloat32 FFX_DNSR_Reflections_LoadDepthHistory(const in FfxInt32x2 coord)
{
	return tx_depthHistory.Load(FfxInt32x3(coord, 0));
}

FfxFloat32 FFX_DNSR_Reflections_LoadDepth(const in FfxInt32x2 coord)
{
	return tx_depthHierarchy.Load(FfxInt32x3(coord, 0));
}

FfxFloat32x2 FFX_DNSR_Reflections_LoadMotionVector(const in FfxInt32x2 coord)
{
	return MotionVectorScale() * tx_motionVectors.Load(FfxInt32x3(coord, 0));
}

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_reflections_reproject.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint gid : SV_GroupID, const in uint2 gtid : SV_GroupThreadID)
{
	Reproject(groupIndex, gid, gtid);
}