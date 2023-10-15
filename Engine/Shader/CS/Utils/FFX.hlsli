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
#if defined(FFX_PREFER_WAVE64) && !defined(_DX11)
#	undef FFX_PREFER_WAVE64
#	define FFX_PREFER_WAVE64 [WaveSize(64)]
#else
#	define FFX_PREFER_WAVE64
#endif
#if defined(_DX11) && defined(FFX_PREFER_WAVE64)
#	undef FFX_PREFER_WAVE64
#endif
#ifndef FFX_PREFER_WAVE64
#	define FFX_PREFER_WAVE64
#endif

SamplerState splr_PointClamp : register(s0);
SamplerState splr_PointMirror : register(s1);
SamplerState splr_LinearClamp : register(s2);
SamplerState splr_ViewspaceDepthTap : register(s3);
SamplerState splr_RealPointClamp : register(s4);

#endif // FFX_CS_HLSLI