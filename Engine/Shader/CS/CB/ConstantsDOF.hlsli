#ifndef CONSTANTS_DOF_CS_HLSLI
#define CONSTANTS_DOF_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_DOF
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

struct DofGlobalVars
{
	uint MaxTileRad;
};

// To correctly use this cbuffer, define 'ZE_DOF_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsDOF
{
	FfxFloat32 CocScale;
	FfxFloat32 CocBias;
	FfxUInt32x2 InputSizeHalf;
	FfxUInt32x2 InputSize;
	FfxFloat32x2 InputSizeHalfRcp;
	FfxFloat32 CocLimit;
	FfxUInt32 MaxRings;
};

CBUFFER(dofConsts, ConstantsDOF, 0, ZE_DOF_CB_RANGE);

FfxFloat32 CocScale()
{
	return cb_dofConsts.CocScale;
}

FfxFloat32 CocBias()
{
	return cb_dofConsts.CocBias;
}

FfxUInt32x2 InputSizeHalf()
{
	return cb_dofConsts.InputSizeHalf;
}

FfxUInt32x2 InputSize()
{
	return cb_dofConsts.InputSize;
}

FfxFloat32x2 InputSizeHalfRcp()
{
	return cb_dofConsts.InputSizeHalfRcp;
}

FfxFloat32 CocLimit()
{
	return cb_dofConsts.CocLimit;
}

FfxUInt32 MaxRings()
{
	return cb_dofConsts.MaxRings;
}

FfxFloat32 FfxDofGetCoc(const in FfxFloat32 depth)
{
    // clamped for perf reasons
	return clamp(CocScale() * depth + CocBias(), -CocLimit(), CocLimit());
}

#include "dof/ffx_dof_common.h"
#include "dof/ffx_dof_resources.h"

#endif // CONSTANTS_DOF_CS_HLSLI