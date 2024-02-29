#define ZE_DENOISER_SHADOWS_1_CB_RANGE 10
#include "CB/ConstantsDenoiserShadows1.hlsli"
#include "GBufferUtils.hlsli"

UAV_EX(raytracerResults, RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV_EX(tileMetadata, RWStructuredBuffer<FfxUInt32>, 1, 1);
UAV2D(reprojectionResults, FfxFloat32x2, 2, 2);
UAV2D(currentMoments, FfxFloat32x3, 3, 3);
TEXTURE_EX(normals, Texture2D<CodedNormalGB>, 0, 4); // External resource format
TEXTURE_EX(prevDepth, Texture2D<FfxFloat32>, 1, 5);
TEXTURE_EX(velocity, Texture2D<MotionGB>, 2, 6); // External resource format
TEXTURE_EX(depth, Texture2D<float>, 3, 7); // External resource format
TEXTURE_EX(previousMoments, Texture2D<FfxFloat32x3>, 4, 8);
TEXTURE_EX(history, Texture2D<FfxFloat32x2>, 5, 9);

FfxUInt32 LoadRaytracedShadowMask(const in FfxUInt32 coord)
{
	return ua_raytracerResults[coord];
}

void StoreMetadata(const in FfxUInt32 coord, const in FfxUInt32 val)
{
	ua_tileMetadata[coord] = val;
}

void StoreReprojectionResults(const in FfxUInt32x2 coord, const in FfxFloat32x2 val)
{
	ua_reprojectionResults[coord] = val;
}

void StoreMoments(const in FfxUInt32x2 coord, const in FfxFloat32x3 val)
{
	ua_currentMoments[coord] = val;
}

FfxFloat32x3 LoadNormals(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)));
}

FfxFloat32 LoadPreviousDepth(const in FfxInt32x2 coord)
{
	return tx_prevDepth.Load(FfxInt32x3(coord, 0)).x;
}

FfxFloat32x2 LoadVelocity(const in FfxInt32x2 coord)
{
	MotionGB velocity = tx_velocity.Load(FfxInt32x3(coord, 0)).rg;
    return velocity * MotionVectorScale();
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

FfxFloat32x3 LoadPreviousMomentsBuffer(const in FfxInt32x2 coord)
{
	return tx_previousMoments.Load(FfxInt32x3(coord, 0)).xyz;
}

FfxFloat32 LoadHistory(const in FfxFloat32x2 coord)
{
	return tx_history.SampleLevel(splr_trilinerClamp, coord, 0).x;
}

#include "WarningGuardOn.hlsli"
#include "denoiser/ffx_denoiser_shadows_tileclassification.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint2 gid : SV_GroupID)
{
	FFX_DNSR_Shadows_TileClassification(groupIndex, gid);
}