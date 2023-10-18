#define ZE_CACAO_CB_RANGE 2
#include "CB/ConstantsCACAO.hlsli"
#include "CommonUtils.hlsli"

UAV_EX(deinterleavedNormals, RWTexture2DArray<FfxFloat32x4>, 0, 0);

void FFX_CACAO_Prepare_StoreNormal(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32x3 normal)
{
	ua_deinterleavedNormals[FfxInt32x3(coord, index)] = FfxFloat32x4(normal, 1.0f);
}

// Stubs not used in this shader version
FfxFloat32x4 FFX_CACAO_Prepare_SampleDepthOffsets(const in FfxFloat32x2 uv) { return 0.0f; }
FfxFloat32x4 FFX_CACAO_Prepare_GatherDepth(const in FfxFloat32x2 uv) { return 0.0f; }
FfxFloat32 FFX_CACAO_Prepare_LoadDepth(const in FfxUInt32x2 coord) { return 0.0f; }
void FFX_CACAO_Prepare_StoreDepthMip0(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip1(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip2(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip3(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepth(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}

#ifdef _CACAO_PREPARE_NORMALS_INPUT
TEXTURE_EX(normals, Texture2D<float2>, 0, 1);

FfxFloat32x3 FFX_CACAO_Prepare_LoadNormal(const in FfxUInt32x2 coord)
{
	float3 normal = DecodeNormal(tx_normals.Load(FfxInt32x3(coord, 0)).xy);
	normal = mul(normal, (float3x3) NormalsWorldToViewspaceMatrix()).xyz;
    // normal = normalize(normal);
	return normal;
}

// Stubs not used in this shader version
FfxFloat32 FFX_CACAO_Prepare_LoadDepthOffset(const in FfxUInt32x2 coord, const in FfxInt32x2 offset) { return 0.0f; }
FfxFloat32x4 FFX_CACAO_Prepare_GatherDepthOffset(const in FfxFloat32x2 uv, const in FfxInt32x2 offset) { return 0.0f; }
#else
TEXTURE_EX(depth, Texture2D<float>, 0, 1);

FfxFloat32 FFX_CACAO_Prepare_LoadDepthOffset(const in FfxUInt32x2 coord, const in FfxInt32x2 offset)
{
	return tx_depth.Load(FfxInt32x3(coord, 0), offset);
}

FfxFloat32x4 FFX_CACAO_Prepare_GatherDepthOffset(const in FfxFloat32x2 uv, const in FfxInt32x2 offset)
{
	return tx_depth.GatherRed(splr_PointClamp, uv, offset);
}

// Stubs not used in this shader version
FfxFloat32x3 FFX_CACAO_Prepare_LoadNormal(const in FfxUInt32x2 coord) { return 0.0f; }
#endif

#include "cacao/ffx_cacao_prepare.h"
#ifdef _CACAO_PREPARE_NORMALS_INPUT
#	define THREAD_WIDTH PREPARE_NORMALS_FROM_INPUT_NORMALS_WIDTH
#	define THREAD_HEIGHT PREPARE_NORMALS_FROM_INPUT_NORMALS_HEIGHT
#	ifdef _CACAO_PREPARE_DOWNSAMPLED
#		define PREPARE_METHOD FFX_CACAO_PrepareDownsampledNormalsFromInputNormals
#	else
#		define PREPARE_METHOD FFX_CACAO_PrepareNativeNormalsFromInputNormals
#	endif
#else
#	define THREAD_WIDTH FFX_CACAO_PREPARE_NORMALS_WIDTH
#	define THREAD_HEIGHT FFX_CACAO_PREPARE_NORMALS_HEIGHT
#	ifdef _CACAO_PREPARE_DOWNSAMPLED
#		define PREPARE_METHOD FFX_CACAO_PrepareDownsampledNormals
#	else
#		define PREPARE_METHOD FFX_CACAO_PrepareNativeNormals
#	endif
#endif

FFX_PREFER_WAVE64
[numthreads(THREAD_WIDTH, THREAD_HEIGHT, 1)]
void main(const uint2 tid : SV_DispatchThreadID)
{
	PREPARE_METHOD(tid);
}