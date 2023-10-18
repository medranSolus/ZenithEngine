#define ZE_CACAO_CB_RANGE 2
#include "CB/ConstantsCACAO.hlsli"

UAV_EX(deinterleavedDepth, RWTexture2DArray<FfxFloat32>, 0, 0);
TEXTURE_EX(depth, Texture2D<float>, 0, 1);

void FFX_CACAO_Prepare_StoreDepth(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val)
{
	ua_deinterleavedDepth[FfxInt32x3(coord, index)] = val;
}

FfxFloat32x4 FFX_CACAO_Prepare_SampleDepthOffsets(const in FfxFloat32x2 uv)
{
    FfxFloat32x4 samples;
    samples.x = tx_depth.SampleLevel(splr_PointClamp, uv, 0.0f, FfxInt32x2(0, 2));
    samples.y = tx_depth.SampleLevel(splr_PointClamp, uv, 0.0f, FfxInt32x2(2, 2));
    samples.z = tx_depth.SampleLevel(splr_PointClamp, uv, 0.0f, FfxInt32x2(2, 0));
    samples.w = tx_depth.SampleLevel(splr_PointClamp, uv, 0.0f, FfxInt32x2(0, 0));
    return samples;
}

FfxFloat32x4 FFX_CACAO_Prepare_GatherDepth(const in FfxFloat32x2 uv)
{
    return tx_depth.GatherRed(splr_PointClamp, uv);
}

FfxFloat32 FFX_CACAO_Prepare_LoadDepth(const in FfxUInt32x2 coord)
{
    return tx_depth.Load(FfxInt32x3(coord, 0));
}

// Stubs not used in this shader version
FfxFloat32 FFX_CACAO_Prepare_LoadDepthOffset(const in FfxUInt32x2 coord, const in FfxInt32x2 offset) { return 0.0f; }
FfxFloat32x4 FFX_CACAO_Prepare_GatherDepthOffset(const in FfxFloat32x2 uv, const in FfxInt32x2 offset) { return 0.0f; }
FfxFloat32x3 FFX_CACAO_Prepare_LoadNormal(const in FfxUInt32x2 coord) { return 0.0f; }
void FFX_CACAO_Prepare_StoreDepthMip0(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip1(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip2(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreDepthMip3(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32 val) {}
void FFX_CACAO_Prepare_StoreNormal(const in FfxUInt32x2 coord, const in FfxUInt32 index, const in FfxFloat32x3 normal) {}

#include "cacao/ffx_cacao_prepare.h"
#ifdef _CACAO_PREPARE_DEPTH_HALF
#	define THREAD_WIDTH FFX_CACAO_PREPARE_DEPTHS_HALF_WIDTH
#	define THREAD_HEIGHT FFX_CACAO_PREPARE_DEPTHS_HALF_HEIGHT
#	ifdef _CACAO_PREPARE_DOWNSAMPLED
#		define PREPARE_METHOD FFX_CACAO_PrepareDownsampledDepths
#	else
#		define PREPARE_METHOD FFX_CACAO_PrepareNativeDepths
#	endif
#else
#	define THREAD_WIDTH FFX_CACAO_PREPARE_DEPTHS_WIDTH
#	define THREAD_HEIGHT FFX_CACAO_PREPARE_DEPTHS_HEIGHT
#	ifdef _CACAO_PREPARE_DOWNSAMPLED
#		define PREPARE_METHOD FFX_CACAO_PrepareDownsampledDepthsHalf
#	else
#		define PREPARE_METHOD FFX_CACAO_PrepareNativeDepthsHalf
#	endif
#endif

FFX_PREFER_WAVE64
[numthreads(THREAD_WIDTH, THREAD_HEIGHT, 1)]
void main(const uint2 tid : SV_DispatchThreadID)
{
	PREPARE_METHOD(tid);
}