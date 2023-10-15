#define ZE_CACAO_CB_RANGE 4
#include "CB/ConstantsCACAO.hlsli"

UAV2D(ssao, FfxFloat32, 0, 0);
TEXTURE_EX(depth, Texture2D<FfxFloat32>, 0, 1);
TEXTURE_EX(deinterleavedDepth, Texture2DArray<FfxFloat32>, 1, 2);
TEXTURE_EX(ping, Texture2DArray<FfxFloat32x2>, 2, 3);

void FFX_CACAO_BilateralUpscale_StoreOutput(const in FfxUInt32x2 coord, const in FfxInt32x2 offset, const in FfxFloat32 val)
{
	ua_ssao[coord + offset] = val;
}

FfxFloat32 FFX_CACAO_BilateralUpscale_SampleSSAOLinear(const in FfxFloat32x2 uv, const in FfxUInt32 index)
{
	return tx_ping.SampleLevel(splr_LinearClamp, FfxFloat32x3(uv, index), 0).x;
}

FfxFloat32 FFX_CACAO_BilateralUpscale_SampleSSAOPoint(const in FfxFloat32x2 uv, const in FfxUInt32 index)
{
	return tx_ping.SampleLevel(splr_PointClamp, FfxFloat32x3(uv, index), 0).x;
}

FfxFloat32x2 FFX_CACAO_BilateralUpscale_LoadSSAO(const in FfxUInt32x2 coord, const in FfxUInt32 index)
{
	return tx_ping.Load(FfxInt32x4(coord, index, 0));
}

FfxFloat32x4 FFX_CACAO_BilateralUpscale_LoadDepths(const in FfxUInt32x2 coord)
{
	FfxFloat32x4 depths;
	depths.x = tx_depth.Load(FfxInt32x3(coord, 0), FfxInt32x2(0, 0));
	depths.y = tx_depth.Load(FfxInt32x3(coord, 0), FfxInt32x2(1, 0));
	depths.z = tx_depth.Load(FfxInt32x3(coord, 0), FfxInt32x2(0, 1));
	depths.w = tx_depth.Load(FfxInt32x3(coord, 0), FfxInt32x2(1, 1));
    return depths;
}

FfxFloat32 FFX_CACAO_BilateralUpscale_LoadDownscaledDepth(const in FfxUInt32x2 coord, const in FfxUInt32 index)
{
	return tx_deinterleavedDepth.Load(FfxInt32x4(coord, index, 0));
}

#include "cacao/ffx_cacao_upscale.h"

FFX_PREFER_WAVE64
[numthreads(FFX_CACAO_BILATERAL_UPSCALE_WIDTH, FFX_CACAO_BILATERAL_UPSCALE_HEIGHT, 1)]
void main(const uint2 tid : SV_DispatchThreadID, const uint2 gtid : SV_GroupThreadID, const uint2 gid : SV_GroupID)
{
	FFX_CACAO_UpscaleBilateral5x5Pass(tid, gtid, gid);
}