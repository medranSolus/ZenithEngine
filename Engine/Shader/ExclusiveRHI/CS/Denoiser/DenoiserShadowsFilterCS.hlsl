#ifdef _ZE_HALF_PRECISION
#	define ZE_DENOISER_SHADOWS_2_CB_RANGE 5
#else
#	define ZE_DENOISER_SHADOWS_2_CB_RANGE 4
#endif
#include "CB/ConstantsDenoiserShadows2.hlsli"
#include "CommonUtils.hlsli"

#ifndef ZE_DENOISER_SHADOWS_FILTER_METHOD
#	define ZE_DENOISER_SHADOWS_FILTER_METHOD DenoiserShadowsFilterPass0
#endif

UAV_EX(tileMetadata, RWStructuredBuffer<FfxUInt32>, 0, 0);
TEXTURE_EX(depth, Texture2D<float>, 0, 2); // External resource format
TEXTURE_EX(normals, Texture2D<float2>, 1, 3); // External resource format
#ifdef _ZE_HALF_PRECISION
TEXTURE_EX(filterInput, Texture2D<FfxFloat16x2>, 2, 4);
#endif

FfxUInt32 LoadTileMetaData(const in FfxUInt32 coord)
{
	return ua_tileMetadata[coord];
}

FfxFloat32 LoadDepth(const in FfxInt32x2 coord)
{
	return tx_depth.Load(FfxInt32x3(coord, 0)).x;
}

FfxBoolean IsShadowReciever(const in FfxUInt32x2 coord)
{
	FfxFloat32 depth = LoadDepth(coord);
	return (depth > 0.0f) && (depth < 1.0f);
}

FfxFloat32x3 LoadNormals(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)).xy);
}

#ifdef _ZE_HALF_PRECISION
FfxFloat16x2 LoadFilterInput(const in FfxUInt32x2 coord)
{
	return (FfxFloat16x2)tx_filterInput.Load(FfxInt32x3(coord, 0)).xy;
}
#endif

#if ZE_DENOISER_SHADOWS_FILTER_METHOD == DenoiserShadowsFilterPass2
UAV2D(output, unorm FfxFloat32x4, 1, 1);

void StoreFilterOutput(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
	ua_output[coord].x = val;
}

// Stubs not used in this shader version
void StoreHistory(const in FfxUInt32x2 coord, const in FfxFloat32x2 val) {}
#else
UAV2D(history, FfxFloat32x2, 1, 1);

void StoreHistory(const in FfxUInt32x2 coord, const in FfxFloat32x2 val)
{
	ua_history[coord] = val;
}

// Stubs not used in this shader version
void StoreFilterOutput(const in FfxUInt32x2 coord, const in FfxFloat32 val) {}
#endif

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_shadows_filter.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint2 gid : SV_GroupID, const in uint2 gtid : SV_GroupThreadID, const in uint2 did : SV_DispatchThreadID)
{
	ZE_DENOISER_SHADOWS_FILTER_METHOD(gid, gtid, did);
}