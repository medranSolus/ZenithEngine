#define ZE_SSSR_CB_RANGE 9
#include "CB/ConstantsSSSR.hlsli"
#include "GBufferUtils.hlsli"

UAV_EX(rayCounter, globallycoherent RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV_EX(rayList, RWStructuredBuffer<FfxUInt32>, 1, 1);
UAV2D(radiance, FfxFloat32x4, 2, 2);
TEXTURE_EX(blueNoise, Texture2D<FfxFloat32x2>, 0, 3);
TEXTURE_EX(depthHierarchy, Texture2D<FfxFloat32>, 1, 4);
TEXTURE_EX(normals, Texture2D<CodedNormalGB>, 2, 5); // External resource format
TEXTURE_EX(roughness, Texture2D<FfxFloat32>, 3, 6);
TEXTURE_EX(color, Texture2D<float4>, 4, 7); // External resource format
TEXTURE_EX(environmentMap, TextureCube, 5, 8); // External resource format

FfxBoolean IsRayIndexValid(const in FfxUInt32 rayIndex)
{
	return rayIndex < ua_rayCounter[1];
}

FfxUInt32 GetRaylist(const in FfxUInt32 rayIndex)
{
	return ua_rayList[rayIndex];
}

void FFX_SSSR_StoreRadiance(const in FfxUInt32x2 coord, const in FfxFloat32x4 radiance)
{
	ua_radiance[coord] = radiance;
}

FfxFloat32x2 FFX_SSSR_SampleRandomVector2D(const in FfxUInt32x2 pixel)
{
	return tx_blueNoise.Load(FfxInt32x3(pixel.xy % 128, 0));
}

FfxFloat32 FFX_SSSR_LoadDepth(const in FfxInt32x2 coord, const in FfxInt32 mip)
{
	return tx_depthHierarchy.Load(FfxInt32x3(coord, mip));
}

FfxFloat32x3 FFX_SSSR_LoadWorldSpaceNormal(const in FfxInt32x2 coord)
{
	return DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)));
}

FfxFloat32 FFX_SSSR_LoadExtractedRoughness(const in FfxInt32x3 coord)
{
	return tx_roughness.Load(coord).x;
}

FfxFloat32x3 FFX_SSSR_LoadInputColor(const in FfxInt32x3 coord)
{
	return tx_color.Load(coord).xyz;
}

FfxFloat32x3 FFX_SSSR_SampleEnvironmentMap(const in FfxFloat32x3 direction, const in FfxFloat32 preceptualRoughness)
{
	FfxFloat32 width;
	FfxFloat32 height;
	tx_environmentMap.GetDimensions(width, height);
	
	FfxInt32 maxMipLevel = FfxInt32(log2(FfxFloat32(width > 0 ? width : 1)));
	FfxFloat32 mip = clamp(preceptualRoughness * FfxFloat32(maxMipLevel), 0.0f, FfxFloat32(maxMipLevel));
	
	return tx_environmentMap.SampleLevel(splr_EnvironmentMap, direction, mip).xyz * IBLFactor();
}

#include "WarningGuardOn.hlsli"
#include "sssr/ffx_sssr_intersect.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const in uint groupIndex : SV_GroupIndex, const in uint gid : SV_GroupID)
{
	Intersect(groupIndex, gid);
}