#ifndef CONSTANTS_FSR1_CS_HLSLI
#define CONSTANTS_FSR1_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_FSR1
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_FSR1_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsFSR1
{
	FfxUInt32x4 const0;
	FfxUInt32x4 const1;
	FfxUInt32x4 const2;
	FfxUInt32x4 const3;
	FfxUInt32x4 sample;
};

CBUFFER(fsr1Consts, ConstantsFSR1, 0, ZE_FSR1_CB_RANGE);

FfxUInt32x4 Const0()
{
	return cb_fsr1Consts.const0;
}
FfxUInt32x4 Const1()
{
	return cb_fsr1Consts.const1;
}
FfxUInt32x4 Const2()
{
	return cb_fsr1Consts.const2;
}
FfxUInt32x4 Const3()
{
	return cb_fsr1Consts.const3;
}
FfxUInt32x4 EASUSample()
{
	return cb_fsr1Consts.sample;
}
FfxUInt32x4 RCasSample()
{
	return cb_fsr1Consts.sample;
}
FfxUInt32x4 RCasConfig()
{
	return cb_fsr1Consts.const0;
}

#include "fsr1/ffx_fsr1_resources.h"

#endif // CONSTANTS_FSR1_CS_HLSLI