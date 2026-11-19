#define ZE_DOF_CB_RANGE 3
#include "CB/ConstantsDOF.hlsli"

UAV_EX(globals, globallycoherent RWStructuredBuffer<DofGlobalVars>, 0, 0);
UAV2D(radius, FfxFloat32x2, 1, 1);
UAV2D(dilatedRadius, FfxFloat32x2, 2, 2);

FfxUInt32 FfxDofGetMaxTileRadius()
{
	return ua_globals[0].MaxTileRad;
}

FfxFloat32x2 FfxDofLoadTileRadius(const in FfxUInt32x2 tile)
{
	return ua_radius[tile];
}

void FfxDofStoreDilatedRadius(const in FfxUInt32x2 coord, const in FfxFloat32x2 dilatedMinMax)
{
	ua_dilatedRadius[coord] = dilatedMinMax;
}

#include "WarningGuardOn.hlsli"
#include "dof/ffx_dof_dilate_depth.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint3 dtid : SV_DispatchThreadID)
{
	FfxDofDilate(dtid.xy, InputSize());
}