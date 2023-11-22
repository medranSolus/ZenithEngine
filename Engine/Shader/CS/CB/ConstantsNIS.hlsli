#ifndef CONSTANTS_NIS_CS_HLSLI
#define CONSTANTS_NIS_CS_HLSLI
#include "Buffers.hlsli"


struct ConstantsNIS
{
	float DetectRatio;
	float DetectThres;
	float MinContrastRatio;
	float RatioNorm;

	float ContrastBoost;
	float Eps;
	float SharpStartY;
	float SharpScaleY;

	float SharpStrengthMin;
	float SharpStrengthScale;
	float SharpLimitMin;
	float SharpLimitScale;

	float ScaleX;
	float ScaleY;

	float _DstNormX;
	float _DstNormY;
	float SrcNormX;
	float SrcNormY;

	uint _InputViewportOriginX;
	uint _InputViewportOriginY;
	uint _InputViewportWidth;
	uint _InputViewportHeight;

	uint _OutputViewportOriginX;
	uint _OutputViewportOriginY;
	uint _OutputViewportWidth;
	uint _OutputViewportHeight;

	float _reserved0;
	float _reserved1;
};

CBUFFER(nisConsts, ConstantsNIS, 0, 4);

// Defines fixing names of cbuffer variables used directly in the shader
#define kDetectRatio cb_nisConsts.DetectRatio
#define kDetectThres cb_nisConsts.DetectThres
#define kMinContrastRatio cb_nisConsts.MinContrastRatio
#define kRatioNorm cb_nisConsts.RatioNorm

#define kContrastBoost cb_nisConsts.ContrastBoost
#define kEps cb_nisConsts.Eps
#define kSharpStartY cb_nisConsts.SharpStartY
#define kSharpScaleY cb_nisConsts.SharpScaleY

#define kSharpStrengthMin cb_nisConsts.SharpStrengthMin
#define kSharpStrengthScale cb_nisConsts.SharpStrengthScale
#define kSharpLimitMin cb_nisConsts.SharpLimitMin
#define kSharpLimitScale cb_nisConsts.SharpLimitScale

#define kScaleX cb_nisConsts.ScaleX
#define kScaleY cb_nisConsts.ScaleY

#define kSrcNormX cb_nisConsts.SrcNormX
#define kSrcNormY cb_nisConsts.SrcNormY

#endif // CONSTANTS_NIS_CS_HLSLI