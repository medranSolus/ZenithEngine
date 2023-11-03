#ifndef CONSTANTS_REACTIVE_CS_HLSLI
#define CONSTANTS_REACTIVE_CS_HLSLI
#include "Buffers.hlsli"
#include "Utils/FFX.hlsli"

// To correctly use this cbuffer, define 'ZE_AUTO_REACTIVE_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsReactive
{
	FfxFloat32 gen_reactive_scale;
	FfxFloat32 gen_reactive_threshold;
	FfxFloat32 gen_reactive_binaryValue;
	FfxUInt32 gen_reactive_flags;
};

CBUFFER(reactiveConsts, ConstantsReactive, 0, ZE_AUTO_REACTIVE_CB_RANGE);

FfxFloat32 GenReactiveScale()
{
	return cb_reactiveConsts.gen_reactive_scale;
}
FfxFloat32 GenReactiveThreshold()
{
	return cb_reactiveConsts.gen_reactive_threshold;
}
FfxFloat32 GenReactiveBinaryValue()
{
	return cb_reactiveConsts.gen_reactive_binaryValue;
}
FfxUInt32 GenReactiveFlags()
{
	return cb_reactiveConsts.gen_reactive_flags;
}

#endif // CONSTANTS_REACTIVE_CS_HLSLI