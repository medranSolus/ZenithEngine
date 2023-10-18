#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"

UAV_EX(ping, RWTexture2DArray<FfxFloat32x2>, 0, 0);
TEXTURE_EX(deinterleavedDepth, Texture2DArray<FfxFloat32>, 0, 1);
TEXTURE_EX(deinterleavedNormals, Texture2DArray<FfxFloat32x4>, 1, 2);

void FFX_CACAO_SSAOGeneration_StoreOutput(const in FfxUInt32x2 coord, const in FfxFloat32x2 val, const in FfxUInt32 layerId)
{
	ua_ping[FfxInt32x3(coord, layerId)] = val;
}

FfxFloat32 FFX_CACAO_SSAOGeneration_SampleViewspaceDepthMip(const in FfxFloat32x2 uv, const in FfxFloat32 mip, const in FfxUInt32 layerId)
{
	return tx_deinterleavedDepth.SampleLevel(splr_ViewspaceDepthTap, FfxFloat32x3(uv, FfxFloat32(layerId)), mip);
}

FfxFloat32x4 FFX_CACAO_SSAOGeneration_GatherViewspaceDepthOffset(const in FfxFloat32x2 uv, const in FfxInt32x2 offset, const in FfxUInt32 layerId)
{
	return tx_deinterleavedDepth.GatherRed(splr_PointMirror, FfxFloat32x3(uv, FfxFloat32(layerId)), offset);
}

FfxFloat32x3 FFX_CACAO_SSAOGeneration_GetNormalPass(const in FfxUInt32x2 coord, const in FfxUInt32 passId)
{
	return tx_deinterleavedNormals[FfxInt32x3(coord, passId)].xyz;
}

#ifdef _CACAO_ADDITIONAL_RESOURCES
#define ZE_CACAO_CB_RANGE 6

TEXTURE_EX(loadCounter, Texture1D<FfxUInt32> , 2, 3);
TEXTURE_EX(pong, Texture2DArray<FfxFloat32x2>, 3, 4);
TEXTURE_EX(importanceMap, Texture2D<FfxFloat32>, 4, 5);

FfxUInt32 FFX_CACAO_SSAOGeneration_GetLoadCounter()
{
	return tx_loadCounter[0];
}

FfxFloat32x2 FFX_CACAO_SSAOGeneration_LoadBasePassSSAOPass(const in FfxUInt32x2 coord, const in FfxUInt32 passId)
{
	return tx_pong.Load(FfxInt32x4(coord, passId, 0)).xy;
}

FfxFloat32 FFX_CACAO_SSAOGeneration_SampleImportance(const in FfxFloat32x2 uv)
{
	return tx_importanceMap.SampleLevel(splr_LinearClamp, uv, 0.0f);
}
#else
#define ZE_CACAO_CB_RANGE 3

// Stubs not used in this shader version
FfxUInt32 FFX_CACAO_SSAOGeneration_GetLoadCounter() { return 0; }
FfxFloat32x2 FFX_CACAO_SSAOGeneration_LoadBasePassSSAOPass(const in FfxUInt32x2 coord, const in FfxUInt32 passId) { return 0.0f; }
FfxFloat32 FFX_CACAO_SSAOGeneration_SampleImportance(const in FfxFloat32x2 uv) { return 0.0f; }
#endif

#include "CB/ConstantsCACAO.hlsli"
#include "cacao/ffx_cacao_ssao_generation.h"
#ifndef _CACAO_GENERATE_METHOD
#	define _CACAO_GENERATE_METHOD FFX_CACAO_GenerateQ0
#endif
#ifndef _CACAO_NORMAL_TILE
#	define THREAD_WIDTH FFX_CACAO_GENERATE_SPARSE_WIDTH
#	define THREAD_HEIGHT FFX_CACAO_GENERATE_SPARSE_HEIGHT
#else
#	define THREAD_WIDTH FFX_CACAO_GENERATE_WIDTH
#	define THREAD_HEIGHT FFX_CACAO_GENERATE_HEIGHT
#endif

FFX_PREFER_WAVE64
[numthreads(THREAD_WIDTH, THREAD_HEIGHT, 1)]
void main(const uint3 tid : SV_DispatchThreadID)
{
	_CACAO_GENERATE_METHOD(tid);
}