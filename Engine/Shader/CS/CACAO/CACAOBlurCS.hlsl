#define ZE_CACAO_CB_RANGE 2
#include "CB/ConstantsCACAO.hlsli"

UAV_EX(pong, RWTexture2DArray<FfxFloat32x2>, 0, 0);
TEXTURE_EX(ping, Texture2DArray<FfxFloat32x2>, 0, 1);

void FFX_CACAO_EdgeSensitiveBlur_StoreOutput(const in FfxUInt32x2 coord, const in FfxFloat32x2 value, const in FfxUInt32 layerId)
{
	ua_pong[FfxInt32x3(coord, layerId)] = value;
}

FfxFloat32x2 FFX_CACAO_EdgeSensitiveBlur_SampleInputOffset(const in FfxFloat32x2 uv, const in FfxInt32x2 offset, const in FfxUInt32 layerId)
{
	return tx_ping.SampleLevel(splr_PointMirror, FfxFloat32x3(uv, FfxFloat32(layerId)), 0.0f, offset);
}

FfxFloat32x2 FFX_CACAO_EdgeSensitiveBlur_SampleInput(const in FfxFloat32x2 uv, const in FfxUInt32 layerId)
{
	return tx_ping.SampleLevel(splr_PointMirror, FfxFloat32x3(uv, FfxFloat32(layerId)), 0.0f);
}

#include "cacao/ffx_cacao_edge_sensitive_blur.h"
#ifndef _CACAO_BLUR_METHOD
#	define _CACAO_BLUR_METHOD FFX_CACAO_EdgeSensitiveBlur1
#endif

FFX_PREFER_WAVE64
[numthreads(FFX_CACAO_BLUR_WIDTH, FFX_CACAO_BLUR_HEIGHT, 1)]
void main(const uint2 tid : SV_GroupThreadID, const uint3 gid : SV_GroupID)
{
	_CACAO_BLUR_METHOD(tid, gid);
}