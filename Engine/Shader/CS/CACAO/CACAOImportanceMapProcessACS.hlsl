#define ZE_CACAO_CB_RANGE 2
#include "CB/ConstantsCACAO.hlsli"

UAV2D(importanceMapPong, FfxFloat32, 0, 0);
TEXTURE_EX(importanceMap, Texture2D<FfxFloat32>, 0, 1);

void FFX_CACAO_Importance_StoreImportanceA(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
	ua_importanceMapPong[coord] = val;
}

FfxFloat32 FFX_CACAO_Importance_SampleImportanceA(const in FfxFloat32x2 uv)
{
	return tx_importanceMap.SampleLevel(splr_LinearClamp, uv, 0.0f);
}

// Stubs not used in this shader version
FfxFloat32x4 FFX_CACAO_Importance_GatherSSAO(const in FfxFloat32x2 uv, const in FfxUInt32 index) { return 0.0f; }
void FFX_CACAO_Importance_StoreImportance(const in FfxUInt32x2 coord, const in FfxFloat32 val) {}
FfxFloat32 FFX_CACAO_Importance_SampleImportanceB(const in FfxFloat32x2 uv) { return 0.0f; }
void FFX_CACAO_Importance_StoreImportanceB(const in FfxUInt32x2 coord, const in FfxFloat32 val) {}
void FFX_CACAO_Importance_LoadCounterInterlockedAdd(const in FfxUInt32 val) {}

#include "WarningGuardOn.hlsli"
#include "cacao/ffx_cacao_importance_map.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(IMPORTANCE_MAP_A_WIDTH, IMPORTANCE_MAP_A_HEIGHT, 1)]
void main(const uint2 tid : SV_DispatchThreadID)
{
	FFX_CACAO_PostprocessImportanceMapA(tid);
}