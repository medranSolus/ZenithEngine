#include "Utils/FFX.hlsli"
#include "Buffers.hlsli"

UAV_EX(rayCounter, globallycoherent RWStructuredBuffer<FfxUInt32>, 0, 0);
UAV_EX(intersectionArgs, RWStructuredBuffer<FfxUInt32>, 1, 1);

void FFX_SSSR_WriteRayCounter(const in FfxUInt32 index, const in FfxUInt32 data)
{
    ua_rayCounter[index] = data;
}

FfxUInt32 FFX_SSSR_GetRayCounter(const in FfxUInt32 index)
{
	return ua_rayCounter[index];
}

void FFX_SSSR_WriteIntersectIndirectArgs(const in FfxUInt32 index, const in FfxUInt32 data)
{
	ua_intersectionArgs[index] = data;
}

#include "WarningGuardOn.hlsli"
#include "sssr/ffx_sssr_prepare_indirect_args.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(1, 1, 1)]
void main()
{
	PrepareIndirectArgs();
}