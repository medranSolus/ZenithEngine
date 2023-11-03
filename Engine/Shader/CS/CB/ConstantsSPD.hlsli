#ifndef CONSTANTS_SPD_CS_HLSLI
#define CONSTANTS_SPD_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"

// To correctly use this cbuffer, define 'ZE_SPD_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsSPD
{
	FfxUInt32 mips;
	FfxUInt32 numWorkGroups;
	FfxUInt32x2 workGroupOffset;
	FfxUInt32x2 renderSize;
};

CBUFFER(spdConsts, ConstantsSPD, 1, ZE_SPD_CB_RANGE);

FfxUInt32 MipCount()
{
	return cb_spdConsts.mips;
}
FfxUInt32 NumWorkGroups()
{
	return cb_spdConsts.numWorkGroups;
}
FfxUInt32x2 WorkGroupOffset()
{
	return cb_spdConsts.workGroupOffset;
}
FfxUInt32x2 SPD_RenderSize()
{
	return cb_spdConsts.renderSize;
}

#endif // CONSTANTS_SPD_CS_HLSLI