#ifndef FFX_CS_HLSLI
#define FFX_CS_HLSLI
#define FFX_CORE_H
#include "ffx_common_types.h"
// Changed how 'FFX_GROUP_MEMORY_BARRIER' is defined to avoid problems with FXC incorrect preprocessor
#include "ffx_core_hlsl.hlsli"
#include "ffx_core_gpu_common.h"
#include "ffx_core_gpu_common_half.h"
#include "ffx_core_portability.h"

#ifndef FFX_HALF
#	define FFX_HALF 0
#endif
// Cannot set wave size for DX11
#if defined(FFX_PREFER_WAVE64) && !defined(_DX11)
#	undef FFX_PREFER_WAVE64
#	define FFX_PREFER_WAVE64 [WaveSize(64)]
#else
// Undef in case it's expanded to number by cmd line
#	undef FFX_PREFER_WAVE64
#	define FFX_PREFER_WAVE64
#endif
// No WaveReadLaneAt() function present below SM6.0
#ifdef _DX11
#	define FFX_SPD_NO_WAVE_OPERATIONS
#endif

// Default options
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

// Upsample currently always uses approximation
#define FFX_FSR2_OPTION_UPSAMPLE_USE_LANCZOS_TYPE 2
// Controls which part of code should use half precission even when requested
#define FFX_FSR2_OPTION_UPSAMPLE_SAMPLERS_USE_DATA_HALF 0
#define FFX_FSR2_OPTION_ACCUMULATE_SAMPLERS_USE_DATA_HALF 0
#define FFX_FSR2_OPTION_REPROJECT_SAMPLERS_USE_DATA_HALF 1
#define FFX_FSR2_OPTION_POSTPROCESSLOCKSTATUS_SAMPLERS_USE_DATA_HALF 0

// Common samplers
SamplerState splr_PointClamp : register(s0);
SamplerState splr_PointMirror : register(s1);
SamplerState splr_LinearClamp : register(s2);
SamplerState splr_ViewspaceDepthTap : register(s3);
SamplerState splr_RealPointClamp : register(s4);

#endif // FFX_CS_HLSLI