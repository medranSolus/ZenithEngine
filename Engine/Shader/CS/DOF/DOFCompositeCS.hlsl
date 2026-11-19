#ifdef FFX_DOF_OPTION_COMBINE_IN_PLACE
#	define ZE_DOF_CB_RANGE 6
#else
#	define ZE_DOF_CB_RANGE 7
#endif
#include "CB/ConstantsDOF.hlsli"

UAV2D(colorOutput, float4, 0, 0);
UAV2D(near, FfxHalfOpt4, 1, 1);
UAV2D(far, FfxHalfOpt4, 2, 2);
TEXTURE_EX(dilatedRadius, Texture2D<FfxFloat32x2>, 0, 3);
TEXTURE_EX(bilatColor, Texture2D<FfxHalfOpt4>, 1, 4);
TEXTURE_EX(depthInput, Texture2D<float>, 2, 5);
#ifndef FFX_DOF_OPTION_COMBINE_IN_PLACE
TEXTURE_EX(colorInput, Texture2D<float4>, 3, 6);
#endif

void FfxDofStoreOutput(const in FfxUInt32x2 coord, const in FfxFloat32x4 color)
{
	ua_colorOutput[coord] = color;
}

FfxFloat32x4 FfxDofLoadFullInput(const in FfxUInt32x2 coord)
{
#ifdef FFX_DOF_OPTION_COMBINE_IN_PLACE
    return ua_colorOutput[coord];
#else
	return tx_colorInput[coord];
#endif
}

FfxHalfOpt4 FfxDofLoadNear(const in FfxUInt32x2 coord)
{
	return ua_near[coord];
}

void FfxDofStoreFar(const in FfxUInt32x2 coord, const in FfxHalfOpt4 color)
{
	ua_far[coord] = color;
}

FfxHalfOpt4 FfxDofLoadFar(const in FfxUInt32x2 coord)
{
	return ua_far[coord];
}

FfxFloat32x2 FfxDofLoadDilatedRadius(const in FfxUInt32x2 coord)
{
	return tx_dilatedRadius[coord];
}

FfxHalfOpt4 FfxDofLoadInput(const in FfxUInt32x2 coord)
{
	return tx_bilatColor[coord];
}

FfxFloat32 FfxDofLoadFullDepth(const in FfxUInt32x2 coord)
{
	return tx_depthInput[coord];
}

// Disable optimized path with wave intrinsic
#ifdef _ZE_COMPILER_FXC
uint WaveGetLaneCount()
{
	return 0;
}

uint WaveGetLaneIndex()
{
	return 0;
}

uint2 WaveReadLaneAt(const in uint2 val, const in uint laneIndex)
{
	return val;
}
#endif

#ifndef FFX_DOF_OPTION_COMBINE_IN_PLACE
#	define FFX_DOF_OPTION_COMBINE_IN_PLACE 0
#endif

#include "WarningGuardOn.hlsli"
#include "dof/ffx_dof_composite.h"
#include "WarningGuardOff.hlsli"

ZE_CS_WAVE64
[numthreads(8, 8, 1)]
void main(const uint3 dtid : SV_DispatchThreadID, const uint3 gtid : SV_GroupThreadID, const uint3 gid : SV_GroupID, const uint gidx : SV_GroupIndex)
{
	FfxDofCombineHalfRes(dtid.xy, gtid.xy, gid.xy, gidx, InputSizeHalf(), InputSize());
}