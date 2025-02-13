#define ZE_SSSR_CB_RANGE 10
#include "CB/ConstantsSSSR.hlsli"
#include "GBufferUtils.hlsli"

UAV_EX(rayCounter, globallycoherent RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV_EX(rayList, RWStructuredBuffer<FfxUInt32>, 1, 1);
UAV_EX(denoiserTileList, RWStructuredBuffer<FfxUInt32>, 2, 2);
UAV2D(radiance, FfxFloat32x4, 3, 3);
UAV2D(roughness, FfxFloat32, 4, 4);
TEXTURE_EX(depthHierarchy, Texture2D<FfxFloat32>, 0, 5);
TEXTURE_EX(varianceHistory, Texture2D<FfxFloat32>, 1, 6);
TEXTURE_EX(normals, Texture2D<CodedNormalGB>, 2, 7); // External resource format
TEXTURE_EX(environmentMap, TextureCube, 3, 8); // External resource format
TEXTURE_EX(materialParams, Texture2D<PackedMaterialGB>, 4, 9); // External resource format

void IncrementRayCounter(const in FfxUInt32 value, out FfxUInt32 orgVal)
{
	InterlockedAdd(ua_rayCounter[0], value, orgVal);
}

void IncrementDenoiserTileCounter(out FfxUInt32 orgVal)
{
	InterlockedAdd(ua_rayCounter[2], 1, orgVal);
}

void StoreRay(const in FfxInt32 index, const in FfxUInt32x2 rayCoord, const in FfxBoolean copyHorizontal, const in FfxBoolean copyVertical, const in FfxBoolean copyDiagonal)
{
	FfxUInt32 ray15bitX = rayCoord.x & 0b111111111111111;
	FfxUInt32 ray14bitY = rayCoord.y & 0b11111111111111;

	FfxUInt32 packed = (copyDiagonal << 31) | (copyVertical << 30) | (copyHorizontal << 29) | (ray14bitY << 15) | (ray15bitX << 0);
	// Store out pixel to trace
	ua_rayList[index] = packed;
}

void StoreDenoiserTile(const in FfxInt32 index, const in FfxUInt32x2 tileCoord)
{
	ua_denoiserTileList[index] = ((tileCoord.y & 0xffffu) << 16) | ((tileCoord.x & 0xffffu) << 0); // Store out pixel to trace
}

void FFX_SSSR_StoreRadiance(const in FfxUInt32x2 coord, const in FfxFloat32x4 radiance)
{
	ua_radiance[coord] = radiance;
}

void StoreExtractedRoughness(const in FfxUInt32x2 coord, const in FfxFloat32 roughness)
{
	ua_roughness[coord] = roughness;
}

FfxBoolean IsReflectiveSurface(const in FfxInt32x2 coord, const in FfxFloat32 roughness)
{
	// Check with far plane
#if FFX_SSSR_OPTION_INVERTED_DEPTH
    return tx_depthHierarchy[coord] > 0.0f;
#else
	return tx_depthHierarchy[coord] < 1.0f;
#endif
}

FfxFloat32 FFX_SSSR_LoadDepth(const in FfxInt32x2 coord, const in FfxInt32 mip)
{
	return tx_depthHierarchy.Load(FfxInt32x3(coord, mip));
}

FfxFloat32 FFX_SSSR_LoadVarianceHistory(const in FfxInt32x3 coord)
{
	return tx_varianceHistory.Load(coord).x;
}

FfxFloat32x3 FFX_SSSR_LoadWorldSpaceNormal(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)));
}

FfxFloat32x3 FFX_SSSR_SampleEnvironmentMap(const in FfxFloat32x3 direction, const in FfxFloat32 preceptualRoughness)
{
    FfxFloat32 width;
	FfxFloat32 height;
	tx_environmentMap.GetDimensions(width, height);
	
	FfxInt32 maxMipLevel = FfxInt32(log2(FfxFloat32(width > 0 ? width : 1)));
    FfxFloat32 mip = clamp(preceptualRoughness * FfxFloat32(maxMipLevel), 0.0, FfxFloat32(maxMipLevel));
	
	return tx_environmentMap.SampleLevel(splr_EnvironmentMap, direction, mip).xyz * IBLFactor();
}

FfxFloat32 LoadRoughnessFromMaterialParametersInput(const in FfxUInt32x3 coord)
{
	float rawRoughness = GetRoughness(tx_materialParams.Load(coord));
    // Roughness is always linear in internal storage
	return rawRoughness * rawRoughness;
}

#include "WarningGuardOn.hlsli"
#include "sssr/ffx_sssr_classify_tiles.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const in uint localGroupIndex : SV_GroupIndex, const in uint3 gid : SV_GroupID)
{
	FfxUInt32x2 gtid = ffxRemapForWaveReduction(localGroupIndex);
	FfxUInt32x2 dtid = gid.xy * 8 + gtid;
	FfxFloat32 roughness = LoadRoughnessFromMaterialParametersInput(FfxInt32x3(dtid, 0));

	ClassifyTiles(dtid, gtid, roughness);

    // Extract only the channel containing the roughness to avoid loading all 4 channels in the follow up passes.
	StoreExtractedRoughness(dtid, roughness);
}