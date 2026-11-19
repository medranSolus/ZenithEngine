#define ZE_DOF_CB_RANGE 3
#include "CB/ConstantsDOF.hlsli"

UAV_EX(globals, globallycoherent RWStructuredBuffer<DofGlobalVars>, 0, 0);
UAV2D(radius, FfxFloat32x2, 1, 1);
TEXTURE_EX(depthInput, Texture2D<float>, 0, 2);

void FfxDofAccumMaxTileRadius(const in FfxUInt32 radius)
{
	uint dummy;
	InterlockedMax(ua_globals[0].MaxTileRad, radius, dummy);
}

void FfxDofStoreTileRadius(const in FfxUInt32x2 tile, const in FfxFloat32x2 radius)
{
	ua_radius[tile] = radius;
}

FfxFloat32x4 FfxDofGatherDepth(const in FfxFloat32x2 coord)
{
	return tx_depthInput.GatherRed(splr_Linear, coord);
}

#ifndef FFX_DOF_OPTION_REVERSE_DEPTH
#	define FFX_DOF_OPTION_REVERSE_DEPTH 0
#endif

#include "WarningGuardOn.hlsli"
#include "dof/ffx_dof_downsample_depth.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint3 dtid : SV_DispatchThreadID)
{
	DownsampleDepth(dtid.xy, InputSize());
}