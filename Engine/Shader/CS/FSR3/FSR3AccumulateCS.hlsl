#if ZE_USE_FFX_API_FSR3_SHADERS
#	define _ZE_FFX_API
#endif
#if FFX_FSR3UPSCALER_OPTION_APPLY_SHARPENING == 0
#	if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#		define ZE_FSR3_CB_RANGE 11
#	else
#		define ZE_FSR3_CB_RANGE 10
#	endif
#elif FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#	define ZE_FSR3_CB_RANGE 10
#else
#	define ZE_FSR3_CB_RANGE 9
#endif
#include "CB/ConstantsFSR3.hlsli"
#include "GBufferUtils.hlsli"

// Setting the correct r anges and slots
#if FFX_FSR3UPSCALER_OPTION_APPLY_SHARPENING == 0
#	define NEW_LOCKS_RANGE 2
#	define INPUT_COLOR_RANGE 3
#	if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
#		define UPSCALED_COLOR_RANGE 4
#		define FARTHES_DEPTH_RANGE 5
#		define LUMA_INSTABILITY_RANGE 6
#		define DILATED_MOTION_VECTORS_RANGE 7
#	else
#		define MOTION_VECTORS_RANGE 4
#		define UPSCALED_COLOR_RANGE 5
#		define FARTHES_DEPTH_RANGE 6
#		define LUMA_INSTABILITY_RANGE 7
#	endif
#	define EXPOSURE_RANGE 8
#	if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#		define LANCZOS_RANGE 9
#		define REACTIVE_MASK_RANGE 10
#	else
#		define REACTIVE_MASK_RANGE 9
#	endif
#else
#	define NEW_LOCKS_RANGE 1
#	define INPUT_COLOR_RANGE 2
#	if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
#		define UPSCALED_COLOR_RANGE 3
#		define FARTHES_DEPTH_RANGE 4
#		define LUMA_INSTABILITY_RANGE 5
#		define DILATED_MOTION_VECTORS_RANGE 6
#	else
#		define MOTION_VECTORS_RANGE 3
#		define UPSCALED_COLOR_RANGE 4
#		define FARTHES_DEPTH_RANGE 5
#		define LUMA_INSTABILITY_RANGE 6
#	endif
#	define EXPOSURE_RANGE 7
#	if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#		define LANCZOS_RANGE 8
#		define REACTIVE_MASK_RANGE 9
#	else
#		define REACTIVE_MASK_RANGE 8
#	endif
#endif

#if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
#	define UPSCALED_COLOR_SLOT 1
#	define FARTHES_DEPTH_SLOT 2
#	define LUMA_INSTABILITY_SLOT 3
#else
#	define UPSCALED_COLOR_SLOT 2
#	define FARTHES_DEPTH_SLOT 3
#	define LUMA_INSTABILITY_SLOT 4
#endif

#if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#	define REACTIVE_MASK_SLOT 7
#else
#	define REACTIVE_MASK_SLOT 6
#endif


UAV2D(upscaledColor, FfxFloat32x4, 0, 0);
#if FFX_FSR3UPSCALER_OPTION_APPLY_SHARPENING == 0
UAV2D(upscaledOutput, float4, 1, 1); // External resource format
#endif
UAV2D(newLocks, unorm FfxFloat32, NEW_LOCKS_RANGE, NEW_LOCKS_RANGE);

TEXTURE_EX(inputColor, Texture2D<FfxFloat32x4>, 0, INPUT_COLOR_RANGE);
#if !FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
TEXTURE_EX(motionVectors, Texture2D<MotionGB>, 1, MOTION_VECTORS_RANGE); // External resource format
#endif
TEXTURE_EX(upscaledColor, Texture2D<FfxFloat32x4>, UPSCALED_COLOR_SLOT, UPSCALED_COLOR_RANGE);
TEXTURE_EX(farthestDepthMip1, Texture2D<FfxFloat32>, FARTHES_DEPTH_SLOT, FARTHES_DEPTH_RANGE);
TEXTURE_EX(lumaInstability, Texture2D<FfxFloat32>, LUMA_INSTABILITY_SLOT, LUMA_INSTABILITY_RANGE);
#if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
TEXTURE_EX(dilatedMotionVectors, Texture2D<FfxFloat32x2>, 4, DILATED_MOTION_VECTORS_RANGE);
#endif
TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 5, EXPOSURE_RANGE);
#if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
TEXTURE_EX(lanczosLut, Texture2D<FfxFloat32>, 6, LANCZOS_RANGE);
#endif
TEXTURE_EX(dilatedReactiveMask, Texture2D<unorm FfxFloat32x4>, REACTIVE_MASK_SLOT, REACTIVE_MASK_RANGE);

void StoreInternalColorAndWeight(const in FfxUInt32x2 pxCoord, FfxFloat32x4 colorAndWeight)
{
	ua_upscaledColor[pxCoord] = colorAndWeight;
}

void StoreUpscaledOutput(const in FfxUInt32x2 pxCoord, FfxFloat32x3 color)
{
#if FFX_FSR3UPSCALER_OPTION_APPLY_SHARPENING == 0
	ua_upscaledOutput[pxCoord] = FfxFloat32x4(color, 1.f);
#endif
}

FfxFloat32 LoadRwNewLocks(const in FfxUInt32x2 pxCoord)
{
	return ua_newLocks[pxCoord];
}

void StoreNewLocks(const in FfxUInt32x2 pxCoord, FfxFloat32 newLock)
{
	ua_newLocks[pxCoord] = newLock;
}

FfxFloat32x3 LoadInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_inputColor[pxCoord].rgb;
}

#if !FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxCoord)
{
	FfxFloat32x2 motionVector = tx_motionVectors[pxCoord];
	motionVector *= MotionVectorScale();

#if FFX_FSR3UPSCALER_OPTION_JITTERED_MOTION_VECTORS
    motionVector -= MotionVectorJitterCancellation();
#endif
	return motionVector;
}
#endif

FfxFloat32x4 LoadHistory(const in FfxUInt32x2 pxCoord)
{
	return tx_upscaledColor[pxCoord];
}

FfxInt32x2 GetFarthestDepthMip1ResourceDimensions()
{
	FfxUInt32 width;
	FfxUInt32 height;
	tx_farthestDepthMip1.GetDimensions(width, height);

	return FfxInt32x2(width, height);
}

FfxFloat32 SampleFarthestDepthMip1(const in FfxFloat32x2 uv)
{
	return tx_farthestDepthMip1.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 SampleLumaInstability(const in FfxFloat32x2 uv)
{
	return tx_lumaInstability.SampleLevel(splr_LinearClamp, uv, 0);
}

#if FFX_FSR3UPSCALER_OPTION_LOW_RESOLUTION_MOTION_VECTORS
FfxFloat32x2 LoadDilatedMotionVector(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedMotionVectors[pxCoord];
}
#endif

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

FfxFloat32 SampleLanczos2Weight(const in FfxFloat32 x)
{
#if FFX_FSR3UPSCALER_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
	return tx_lanczosLut.SampleLevel(splr_LinearClamp, FfxFloat32x2(x / 2, 0.5f), 0);
#else
	return 0.0f;
#endif
}

FfxFloat32x4 SampleDilatedReactiveMasks(const in FfxFloat32x2 uv)
{
	return tx_dilatedReactiveMask.SampleLevel(splr_LinearClamp, uv, 0);
}

#include "WarningGuardOn.hlsli"
#ifdef _ZE_FFX_API
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_sample.h"
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_upsample.h"
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_reproject.h"
#	include "upscalers/fsr3/include/gpu/fsr3upscaler/ffx_fsr3upscaler_accumulate.h"
#else
#	include "fsr3upscaler/ffx_fsr3upscaler_sample.h"
#	include "fsr3upscaler/ffx_fsr3upscaler_upsample.h"
#	include "fsr3upscaler/ffx_fsr3upscaler_reproject.h"
#	include "fsr3upscaler/ffx_fsr3upscaler_accumulate.h"
#endif
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint2 dtid : SV_DispatchThreadID)
{
	Accumulate(dtid);
}