#if ZE_USE_FFX_API_FSR3_SHADERS
#	define _ZE_FFX_API
#endif
#define ZE_FSR3_CB_RANGE 2
#include "CB/ConstantsFSR3.hlsli"

UAV2D(shadingChange, FfxFloat32, 0, 0);
TEXTURE_EX(spdMips, Texture2D<FfxFloat32x2>, 0, 1); // External resource format

void StoreShadingChange(const in FfxUInt32x2 pxCoord, const in FfxFloat32 shadingChange)
{
	ua_shadingChange[pxCoord] = shadingChange;
}

FfxInt32x2 GetSPDMipDimensions(const in FfxUInt32 mipLevel)
{
	FfxUInt32 width;
	FfxUInt32 height;
	FfxUInt32 levels;
	tx_spdMips.GetDimensions(mipLevel, width, height, levels);

	return FfxInt32x2(width, height);
}

FfxFloat32x2 SampleSPDMipLevel(const in FfxFloat32x2 uv, const in FfxUInt32 mipLevel)
{
	return tx_spdMips.SampleLevel(splr_LinearClamp, uv, mipLevel);
}

#include "WarningGuardOn.hlsli"
#ifdef _ZE_FFX_API
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_shading_change.h"
#else
#	include "fsr3upscaler/ffx_fsr3upscaler_shading_change.h"
#endif
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	ShadingChange(dtid);
}