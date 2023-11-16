#if FFX_FSR2_OPTION_APPLY_SHARPENING
#	if FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#		define ZE_FSR2_CB_RANGE 13
#	else
#		define ZE_FSR2_CB_RANGE 12
#	endif
#elif FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
#	define ZE_FSR2_CB_RANGE 14
#else
#	define ZE_FSR2_CB_RANGE 13
#endif
#include "CB/ConstantsFSR2.hlsli"

UAV2D(upscaledColor, FfxFloat32x4, 0, 0);
UAV2D(lockStatus, unorm FfxFloat32x2, 1, 1);
UAV2D(newLocks, unorm FfxFloat32, 2, 2);
UAV2D(lumaHistory, FfxFloat32x4, 3, 3);

#if FFX_FSR2_OPTION_APPLY_SHARPENING == 0
#	define EXPOSURE_RANGE 5
#	define MOTION_VECTORS_RANGE 6
#	define UPSCALED_COLOR_RANGE 7
#	define LOCK_STATUS_RANGE 8
#	define INPUT_COLOR_RANGE 9
#	define LUMA_HISTORY_RANGE 10
#	define IMG_MIPS_RANGE 11
#	define REACTIVE_MASK_RANGE 12
#	define LANCZOS_RANGE 13

UAV2D(upscaledOutput, float4, 4, 4); // External resource format

void StoreUpscaledOutput(const in FfxUInt32x2 pxCoord, const in FfxFloat32x3 color)
{
	ua_upscaledOutput[pxCoord] = FfxFloat32x4(color, 1.f);
}
#else
#	define EXPOSURE_RANGE 4
#	define MOTION_VECTORS_RANGE 5
#	define UPSCALED_COLOR_RANGE 6
#	define LOCK_STATUS_RANGE 7
#	define INPUT_COLOR_RANGE 8
#	define LUMA_HISTORY_RANGE 9
#	define IMG_MIPS_RANGE 10
#	define REACTIVE_MASK_RANGE 11
#	define LANCZOS_RANGE 12

// Stub not used in this shader version
void StoreUpscaledOutput(const in FfxUInt32x2 pxCoord, const in FfxFloat32x3 color) {}
#endif

TEXTURE_EX(exposure, Texture2D<FfxFloat32x2>, 0, EXPOSURE_RANGE);
TEXTURE_EX(upscaledColor, Texture2D<FfxFloat32x4>, 2, UPSCALED_COLOR_RANGE);
TEXTURE_EX(lockStatus, Texture2D<unorm FfxFloat32x2>, 3, LOCK_STATUS_RANGE);
TEXTURE_EX(inputColor, Texture2D<FfxFloat32x4>, 4, INPUT_COLOR_RANGE);
TEXTURE_EX(lumaHistory, Texture2D<unorm FfxFloat32x4>, 5, LUMA_HISTORY_RANGE);
TEXTURE_EX(imgMips, Texture2D<FfxFloat32>, 6, IMG_MIPS_RANGE);
TEXTURE_EX(dilatedReactiveMask, Texture2D<unorm FfxFloat32x2>, 7, REACTIVE_MASK_RANGE);

#if FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
TEXTURE_EX(dilatedMotionVectors, Texture2D<FfxFloat32x2>, 1, MOTION_VECTORS_RANGE);

FfxFloat32x2 LoadDilatedMotionVector(const in FfxUInt32x2 pxCoord)
{
	return tx_dilatedMotionVectors[pxCoord].xy;
}
#else
TEXTURE_EX(motionVectors, Texture2D<float2>, 1, MOTION_VECTORS_RANGE); // External resource format

FfxFloat32x2 LoadInputMotionVector(const in FfxUInt32x2 pxDilatedMotionVectorPos)
{
	FfxFloat32x2 srcMotionVector = tx_motionVectors[pxDilatedMotionVectorPos].xy;
	FfxFloat32x2 uvMotionVector = srcMotionVector * MotionVectorScale();

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    uvMotionVector -= MotionVectorJitterCancellation();
#endif
	return uvMotionVector;
}
#endif

#if FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE == 1
TEXTURE_EX(lanczosLut, Texture2D<FfxFloat32>, 8, LANCZOS_RANGE);

FfxFloat32 SampleLanczos2Weight(const in FfxFloat32 x)
{
	return tx_lanczosLut.SampleLevel(splr_LinearClamp, FfxFloat32x2(x / 2, 0.5f), 0);
}
#else
// Stub not used in this shader version
FfxFloat32 SampleLanczos2Weight(const in FfxFloat32 x) { return 0.0f; }
#endif

void StoreInternalColorAndWeight(const in FfxUInt32x2 pxCoord, const in FfxFloat32x4 colorAndWeight)
{
	ua_upscaledColor[pxCoord] = colorAndWeight;
}

void StoreLockStatus(const in FfxUInt32x2 pxCoord, const in FfxFloat32x2 lockStatus)
{
	ua_lockStatus[pxCoord] = lockStatus;
}

void StoreNewLocks(const in FfxUInt32x2 pxCoord, const in FfxFloat32 newLock)
{
	ua_newLocks[pxCoord] = newLock;
}

FfxFloat32 LoadRwNewLocks(const in FfxUInt32x2 pxCoord)
{
	return ua_newLocks[pxCoord];
}

void StoreLumaHistory(const in FfxUInt32x2 pxCoord, const in FfxFloat32x4 lumaHistory)
{
	ua_lumaHistory[pxCoord] = lumaHistory;
}

FfxFloat32 Exposure()
{
	FfxFloat32 exposure = tx_exposure[FfxUInt32x2(0, 0)].x;

	if (exposure == 0.0f)
		exposure = 1.0f;
	return exposure;
}

FfxFloat32x4 LoadHistory(const in FfxUInt32x2 pxHistory)
{
	return tx_upscaledColor[pxHistory];
}

FfxFloat32x2 LoadLockStatus(const in FfxUInt32x2 pxCoord)
{
	return tx_lockStatus[pxCoord];
}

FfxFloat32x2 SampleLockStatus(const in FfxFloat32x2 uv)
{
	return tx_lockStatus.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32x3 LoadPreparedInputColor(const in FfxUInt32x2 pxCoord)
{
	return tx_inputColor[pxCoord].xyz;
}

FfxFloat32 SampleDepthClip(const in FfxFloat32x2 uv)
{
	return tx_inputColor.SampleLevel(splr_LinearClamp, uv, 0).w;
}

FfxFloat32x4 SampleLumaHistory(const in FfxFloat32x2 uv)
{
	return tx_lumaHistory.SampleLevel(splr_LinearClamp, uv, 0);
}

FfxFloat32 LoadMipLuma(const in FfxUInt32x2 pxCoord, const in FfxUInt32 mipLevel)
{
	return tx_imgMips.mips[mipLevel][pxCoord];
}

FfxFloat32 SampleMipLuma(const in FfxFloat32x2 uv, const in FfxUInt32 mipLevel)
{
	return tx_imgMips.SampleLevel(splr_LinearClamp, uv, mipLevel);
}

FfxFloat32x2 SampleDilatedReactiveMasks(const in FfxFloat32x2 uv)
{
	return tx_dilatedReactiveMask.SampleLevel(splr_LinearClamp, uv, 0);
}

#include "WarningGuardOn.hlsli"
#include "fsr2/ffx_fsr2_sample.h"
#include "fsr2/ffx_fsr2_upsample.h"
#include "fsr2/ffx_fsr2_postprocess_lock_status.h"
#include "fsr2/ffx_fsr2_reproject.h"
#include "fsr2/ffx_fsr2_accumulate.h"
#include "WarningGuardOff.hlsli"
#define THREAD_WIDTH 8
#define THREAD_HEIGHT 8

ZE_CS_WAVE64
[numthreads(THREAD_WIDTH, THREAD_HEIGHT, 1)]
void main(uint2 gid : SV_GroupID, const uint2 gtid : SV_GroupThreadID)
{
	const uint groupRows = (uint(DisplaySize().y) + THREAD_HEIGHT - 1) / THREAD_HEIGHT;
	gid.y = groupRows - gid.y - 1;

	const uint2 dtid = gid * uint2(THREAD_WIDTH, THREAD_HEIGHT) + gtid;
	Accumulate(dtid);
}