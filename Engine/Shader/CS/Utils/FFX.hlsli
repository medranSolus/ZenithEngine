#ifndef FFX_CS_HLSLI
#define FFX_CS_HLSLI
#define FFX_CORE_H

#ifdef _ZE_HALF_PRECISION
#	define FFX_HALF 1
#else
#	define FFX_HALF 0
#endif

// Cannot set wave size for DX11
#if defined(_ZE_PREFER_WAVE64) && !defined(_ZE_API_DX11) && ZE_HLSL_6_6
#	define ZE_CS_WAVE64 [WaveSize(64)]
#else
#	define ZE_CS_WAVE64
#endif
// No WaveReadLaneAt() function present below SM6.0
#ifdef _ZE_API_DX11
#	define FFX_SPD_NO_WAVE_OPERATIONS 1
#endif
// Wave operations only available for SM 6.6+
#if defined(_ZE_API_DX12) || defined(_ZE_API_VK)
#	define FFX_WAVE ZE_HLSL_6_6
#endif

// Default options
#ifndef FFX_FSR1_OPTION_APPLY_RCAS
#	define FFX_FSR1_OPTION_APPLY_RCAS 0
#endif
#ifndef FFX_FSR1_OPTION_SRGB_CONVERSIONS
#	define FFX_FSR1_OPTION_SRGB_CONVERSIONS 0
#endif
#ifndef FFX_FSR1_OPTION_RCAS_PASSTHROUGH_ALPHA
#	define FFX_FSR1_OPTION_RCAS_PASSTHROUGH_ALPHA 0
#endif
#ifndef FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
#	define FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS 0
#endif
#ifndef FFX_FSR2_OPTION_HDR_COLOR_INPUT
#	define FFX_FSR2_OPTION_HDR_COLOR_INPUT 0
#endif
#ifndef FFX_FSR2_OPTION_APPLY_SHARPENING
#	define FFX_FSR2_OPTION_APPLY_SHARPENING 0
#endif
// Only reproject differs between reference (default) and LUT
#ifndef FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE
#   define FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE 0
#endif

#define FFX_SSSR_OPTION_INVERTED_DEPTH 1
// Upsample currently always uses approximation
#define FFX_FSR2_OPTION_UPSAMPLE_USE_LANCZOS_TYPE 2
// Controls which part of code should use half precission even when requested
#define FFX_FSR2_OPTION_UPSAMPLE_SAMPLERS_USE_DATA_HALF 0
#define FFX_FSR2_OPTION_ACCUMULATE_SAMPLERS_USE_DATA_HALF 0
#define FFX_FSR2_OPTION_REPROJECT_SAMPLERS_USE_DATA_HALF 1
#define FFX_FSR2_OPTION_POSTPROCESSLOCKSTATUS_SAMPLERS_USE_DATA_HALF 0

#include "WarningGuardOn.hlsli"
#include "ffx_common_types.h"
// Changed how 'FFX_GROUP_MEMORY_BARRIER' is defined to avoid problems with FXC incorrect preprocessor
#include "ffx_core_hlsl.hlsli"
#include "ffx_core_gpu_common.h"
#include "ffx_core_gpu_common_half.h"
#include "ffx_core_portability.h"
#include "WarningGuardOff.hlsli"

#endif // FFX_CS_HLSLI