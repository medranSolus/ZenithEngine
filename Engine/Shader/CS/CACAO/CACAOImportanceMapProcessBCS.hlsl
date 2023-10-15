#define ZE_CACAO_CB_RANGE 3
#include "CB/ConstantsCACAO.hlsli"

UAV_EX(loadCounter, RWTexture1D<FfxUInt32>, 0, 0);
UAV2D(importanceMap, FfxFloat32, 1, 1);
TEXTURE_EX(pong, Texture2D<FfxFloat32>, 0, 2);

FfxFloat32x4 FFX_CACAO_Importance_GatherSSAO(const in FfxFloat32x2 uv, const in FfxUInt32 index)
{
	return 0.0f;
}

void FFX_CACAO_Importance_StoreImportance(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
}

FfxFloat32 FFX_CACAO_Importance_SampleImportanceA(const in FfxFloat32x2 uv)
{
	return 0.0f;
}

void FFX_CACAO_Importance_StoreImportanceA(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
}

FfxFloat32 FFX_CACAO_Importance_SampleImportanceB(const in FfxFloat32x2 uv)
{
	return tx_pong.SampleLevel(splr_LinearClamp, uv, 0.0f);
}

void FFX_CACAO_Importance_StoreImportanceB(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
	ua_importanceMap[coord] = val;
}

void FFX_CACAO_Importance_LoadCounterInterlockedAdd(const in FfxUInt32 val)
{
	InterlockedAdd(ua_loadCounter[0], val);
}

#include "cacao/ffx_cacao_importance_map.h"

FFX_PREFER_WAVE64
[numthreads(IMPORTANCE_MAP_B_WIDTH, IMPORTANCE_MAP_B_HEIGHT, 1)]
void main(const uint2 tid : SV_DispatchThreadID)
{
	FFX_CACAO_PostprocessImportanceMapB(tid);
}