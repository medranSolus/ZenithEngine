#define ZE_FSR3_CB_RANGE 5
#include "CB/ConstantsFSR3.hlsli"

UAV2D(upscaled, float4, 0, 0); // External resource format
TEXTURE_EX(upscaledHistory, Texture2D<FfxFloat32x4>, 0, 1);
TEXTURE_EX(dilatedMotionVectors, Texture2D<FfxFloat32x2>, 1, 2);
TEXTURE_EX(dilatedDepth, Texture2D<FfxFloat32>, 2, 3);
TEXTURE_EX(dilatedReactiveMask, Texture2D<unorm FfxFloat32x4>, 3, 4);

void StoreUpscaledOutput(const in FfxUInt32x2 pxPosition, const in FfxFloat32x3 color)
{
	ua_upscaled[pxPosition] = FfxFloat32x4(color, 1.0f);
}

FfxFloat32x4 SampleHistory(const in FfxFloat32x2 uv)
{
	return tx_upscaledHistory.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32x2 SampleDilatedMotionVector(const in FfxFloat32x2 uv)
{
	return tx_dilatedMotionVectors.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 SampleDilatedDepth(const in FfxFloat32x2 uv)
{
	return tx_dilatedDepth.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32x4 SampleDilatedReactiveMasks(const in FfxFloat32x2 uv)
{
	return tx_dilatedReactiveMask.SampleLevel(splr_LinearClamp, uv, 0);
}

#include "WarningGuardOn.hlsli"
#include "fsr3upscaler/ffx_fsr3upscaler_debug_view.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	DebugView(dtid);
}