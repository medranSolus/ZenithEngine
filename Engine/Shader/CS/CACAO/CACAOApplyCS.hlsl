#define ZE_CACAO_CB_RANGE 2
#include "CB/ConstantsCACAO.hlsli"

UAV2D(ssao, uint, 0, 0);
TEXTURE_EX(ping, Texture2DArray<FfxFloat32x2>, 0, 1);

void FFX_CACAO_Apply_StoreOutput(const in FfxUInt32x2 coord, const in FfxFloat32 val)
{
	ua_ssao[coord] = val * 255.0f;
}

FfxFloat32x2 FFX_CACAO_Apply_LoadSSAOPass(const in FfxUInt32x2 coord, const in FfxUInt32 passId)
{
	return tx_ping.Load(FfxInt32x4(coord, passId, 0));
}

FfxFloat32 FFX_CACAO_Apply_SampleSSAOUVPass(const in FfxFloat32x2 uv, const in FfxUInt32 passId)
{
	return tx_ping.SampleLevel(splr_LinearClamp, FfxFloat32x3(uv, passId), 0.0f).x;
}

#include "WarningGuardOn.hlsli"
#include "cacao/ffx_cacao_apply.h"
#include "WarningGuardOff.hlsli"

#ifndef ZE_CACAO_APPLY_METHOD
#	define ZE_CACAO_APPLY_METHOD FFX_CACAO_Apply
#endif

ZE_CS_WAVE64
[numthreads(FFX_CACAO_APPLY_WIDTH, FFX_CACAO_APPLY_HEIGHT, 1)]
void main(const uint2 pixCoord : SV_DispatchThreadID)
{
	ZE_CACAO_APPLY_METHOD(pixCoord);
}