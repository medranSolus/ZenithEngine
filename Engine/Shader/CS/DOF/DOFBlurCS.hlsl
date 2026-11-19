#define ZE_DOF_CB_RANGE 5
#define _ZE_ENABLE_WAVE_EMULATION
#define ZE_EMULATED_WAVE_GTID_NAME id
#include "CB/ConstantsDOF.hlsli"

UAV_EX(globals, globallycoherent RWStructuredBuffer<DofGlobalVars>, 0, 0);
UAV2D(near, FfxHalfOpt4, 1, 1);
UAV2D(far, FfxHalfOpt4, 2, 2);
TEXTURE_EX(dilatedRadius, Texture2D<FfxFloat32x2>, 0, 3);
TEXTURE_EX(bilatColor, Texture2D<FfxHalfOpt4>, 1, 4);

void FfxDofResetMaxTileRadius()
{
	ua_globals[0].MaxTileRad = 0;
}

void FfxDofStoreNear(const in FfxUInt32x2 coord, const in FfxHalfOpt4 color)
{
	ua_near[coord] = color;
}

void FfxDofStoreFar(const in FfxUInt32x2 coord, const in FfxHalfOpt4 color)
{
	ua_far[coord] = color;
}

FfxFloat32x2 FfxDofSampleDilatedRadius(const in FfxUInt32x2 coord)
{
	return tx_dilatedRadius.SampleLevel(splr_Linear, float2(coord) * InputSizeHalfRcp(), 0);
}

FfxHalfOpt4 FfxDofLoadInput(const in FfxUInt32x2 coord)
{
	return tx_bilatColor[coord];
}

FfxHalfOpt4 FfxDofSampleInput(const in FfxFloat32x2 coord, const in FfxUInt32 mip)
{
	return tx_bilatColor.SampleLevel(splr_Point, coord, mip);
}

#ifdef _ZE_COMPILER_FXC
// ffx_dof_blur.h have tons of useless warnings with potential division by 0.0f
#	pragma warning(disable : 4008)
#endif

#ifndef FFX_DOF_OPTION_MAX_RING_MERGE_LOG
#	define FFX_DOF_OPTION_MAX_RING_MERGE_LOG 0
#endif

#include "WarningGuardOn.hlsli"
#include "dof/ffx_dof_blur.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint3 dtid : SV_DispatchThreadID)
{
	FfxDofBlur(dtid.xy, InputSizeHalf());
}