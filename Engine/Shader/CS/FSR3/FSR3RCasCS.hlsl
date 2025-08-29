#if ZE_USE_FFX_API_FSR3_SHADERS
#	define _ZE_FFX_API
#endif
#define ZE_RCAS_CB_RANGE 3
#define ZE_RCAS_SLOT 0
#include "CB/ConstantsRCAS.hlsli"

UAV2D(upscaled, float4, 0, 0); // External resource format
TEXTURE_EX(rcasInput, Texture2D<FfxFloat32x4>, 0, 1);
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 1, 2);

void StoreUpscaledOutput(const in FfxUInt32x2 pxCoord, const in FfxFloat32x3 color)
{
	ua_upscaled[pxCoord] = FfxFloat32x4(color, 1.0f);
}

FfxFloat32x4 LoadRCAS_Input(const in FfxInt32x2 pxCoord)
{
	return tx_rcasInput[pxCoord];
}

FfxFloat32 Exposure()
{
	FfxFloat32 exposure = tx_exposure[FfxUInt32x2(0, 0)].x;

#if _ZE_PLATFORM_XBOX_SCARLET
    if (exposure < 0.000030517578/** 2^-15 */)
        exposure = 1.0f;
#else
	if (exposure == 0.0f)
		exposure = 1.0f;
#endif
	return exposure;
}

#include "WarningGuardOn.hlsli"
#ifdef _ZE_FFX_API
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_rcas.h"
#else
#	include "fsr3upscaler/ffx_fsr3upscaler_rcas.h"
#endif
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(64, 1, 1)]
void main(const uint3 ltid : SV_GroupThreadID, const uint3 wgid : SV_GroupID, const uint3 dtid : SV_DispatchThreadID)
{
	RCAS(ltid, wgid, dtid);
}