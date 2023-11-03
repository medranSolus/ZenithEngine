#ifndef CONSTANTS_RCAS_CS_HLSLI
#define CONSTANTS_RCAS_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"

// To correctly use this cbuffer, define 'ZE_AUTO_RCAS_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsRCAS
{
	FfxUInt32x4 rcasConfig;
};

CBUFFER(rcasConsts, ConstantsRCAS, 1, ZE_AUTO_RCAS_CB_RANGE);

FfxUInt32x4 RCASConfig()
{
	return cb_rcasConsts.rcasConfig;
}

#endif // CONSTANTS_RCAS_CS_HLSLI