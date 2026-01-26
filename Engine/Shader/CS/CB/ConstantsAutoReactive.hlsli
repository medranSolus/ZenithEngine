#ifndef CONSTANTS_AUTO_REACTIVE_CS_HLSLI
#define CONSTANTS_AUTO_REACTIVE_CS_HLSLI
#include "Utils/FFX.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_AUTO_REACTIVE_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsAutoReactive
{
	FfxFloat32 fTcThreshold; // 0.1 is a good starting value, lower will result in more TC pixels
	FfxFloat32 fTcScale;
	FfxFloat32 fReactiveScale;
	FfxFloat32 fReactiveMax;
};

CBUFFER(autoReactConsts, ConstantsAutoReactive, 1, ZE_AUTO_REACTIVE_CB_RANGE);

FfxFloat32 TcThreshold()
{
	return cb_autoReactConsts.fTcThreshold;
}
FfxFloat32 TcScale()
{
	return cb_autoReactConsts.fTcScale;
}
FfxFloat32 ReactiveScale()
{
	return cb_autoReactConsts.fReactiveScale;
}
FfxFloat32 ReactiveMax()
{
	return cb_autoReactConsts.fReactiveMax;
}

#endif // CONSTANTS_AUTO_REACTIVE_CS_HLSLI