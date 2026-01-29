#ifndef TONEMAP_PARAMS_GT7_PS_HLSLI
#define TONEMAP_PARAMS_GT7_PS_HLSLI
#include "Buffers.hlsli"

struct Exposure
{
	float Val;
};

struct TonemapParams
{
	float ParamA;
	float ParamB;
	float ParamC;
	float MidPoint;
	float ToeStrength;
	float ShoulderTreshold;
	float FadeStart;
	float FadeEnd;
	float LuminanceTargetUCS;
	float LuminanceTarget;
	float BlendRatio;
	float CorrectionSDR;
};

CBUFFER(params, TonemapParams, 0, 1);
CONSTANT(exposure, Exposure, 1);

#endif // TONEMAP_PARAMS_GT7_PS_HLSLI