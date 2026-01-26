#ifndef CONSTANTS_CACAO_CS_HLSLI
#define CONSTANTS_CACAO_CS_HLSLI
#include "Utils/FFX.hlsli"
#define _ZE_FFX_CACAO
#include "Utils/FfxSamplers.hlsli"
#include "Buffers.hlsli"

// To correctly use this cbuffer, define 'ZE_CACAO_CB_RANGE' to indicate binding range used for constant buffer
struct ConstantsCACAO
{
	FfxFloat32x2 DepthUnpackConsts;
	FfxFloat32x2 CameraTanHalfFOV;

	FfxFloat32x2 NDCToViewMul;
	FfxFloat32x2 NDCToViewAdd;

	FfxFloat32x2 DepthBufferUVToViewMul;
	FfxFloat32x2 DepthBufferUVToViewAdd;

	FfxFloat32 EffectRadius; // world (viewspace) maximum size of the shadow
	FfxFloat32 EffectShadowStrength; // global strength of the effect (0 - 5)
	FfxFloat32 EffectShadowPow;
	FfxFloat32 EffectShadowClamp;

	FfxFloat32 EffectFadeOutMul; // effect fade out from distance (ex. 25)
	FfxFloat32 EffectFadeOutAdd; // effect fade out to distance   (ex. 100)
	FfxFloat32 EffectHorizonAngleThreshold; // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
	FfxFloat32 EffectSamplingRadiusNearLimitRec; // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

	FfxFloat32 DepthPrecisionOffsetMod;
	FfxFloat32 NegRecEffectRadius; // -1.0 / EffectRadius
	FfxFloat32 LoadCounterAvgDiv; // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
	FfxFloat32 AdaptiveSampleCountLimit;

	FfxFloat32 InvSharpness;
	FfxInt32 BlurNumPasses;
	FfxFloat32 BilateralSigmaSquared;
	FfxFloat32 BilateralSimilarityDistanceSigma;

	FfxFloat32x4 PatternRotScaleMatrices[4][5];

	FfxFloat32 NormalsUnpackMul;
	FfxFloat32 NormalsUnpackAdd;
	FfxFloat32 DetailAOStrength;
	FfxFloat32 Dummy0;

	FfxFloat32x2 SSAOBufferDimensions;
	FfxFloat32x2 SSAOBufferInverseDimensions;

	FfxFloat32x2 DepthBufferDimensions;
	FfxFloat32x2 DepthBufferInverseDimensions;

	FfxInt32x2 DepthBufferOffset;
	FfxFloat32x4 PerPassFullResUVOffset[4];

	FfxFloat32x2 OutputBufferDimensions;
	FfxFloat32x2 OutputBufferInverseDimensions;

	FfxFloat32x2 ImportanceMapDimensions;
	FfxFloat32x2 ImportanceMapInverseDimensions;

	FfxFloat32x2 DeinterleavedDepthBufferDimensions;
	FfxFloat32x2 DeinterleavedDepthBufferInverseDimensions;

	FfxFloat32x2 DeinterleavedDepthBufferOffset;
	FfxFloat32x2 DeinterleavedDepthBufferNormalisedOffset;

	float4x4 NormalsWorldToViewspaceMatrix;
};

CBUFFER(cacaoConsts, ConstantsCACAO, 0, ZE_CACAO_CB_RANGE);

// Accessors for CACAO effect interfaces
FfxFloat32x2 DepthUnpackConsts()
{
	return cb_cacaoConsts.DepthUnpackConsts;
}
FfxFloat32x2 CameraTanHalfFOV()
{
    return cb_cacaoConsts.CameraTanHalfFOV;
}
FfxFloat32x2 NDCToViewMul()
{
	return cb_cacaoConsts.NDCToViewMul;
}
FfxFloat32x2 NDCToViewAdd()
{
    return cb_cacaoConsts.NDCToViewAdd;
}
FfxFloat32x2 DepthBufferUVToViewMul()
{
    return cb_cacaoConsts.DepthBufferUVToViewMul;
}
FfxFloat32x2 DepthBufferUVToViewAdd()
{
    return cb_cacaoConsts.DepthBufferUVToViewAdd;
}
FfxFloat32 EffectRadius()
{
    return cb_cacaoConsts.EffectRadius;
}
FfxFloat32 EffectShadowStrength()
{
    return cb_cacaoConsts.EffectShadowStrength;
}
FfxFloat32 EffectShadowPow()
{
    return cb_cacaoConsts.EffectShadowPow;
}
FfxFloat32 EffectShadowClamp()
{
    return cb_cacaoConsts.EffectShadowClamp;
}
FfxFloat32 EffectFadeOutMul()
{
    return cb_cacaoConsts.EffectFadeOutMul;
}
FfxFloat32 EffectFadeOutAdd()
{
    return cb_cacaoConsts.EffectFadeOutAdd;
}
FfxFloat32 EffectHorizonAngleThreshold()
{
    return cb_cacaoConsts.EffectHorizonAngleThreshold;
}
FfxFloat32 EffectSamplingRadiusNearLimitRec()
{
    return cb_cacaoConsts.EffectSamplingRadiusNearLimitRec;
}
FfxFloat32 DepthPrecisionOffsetMod()
{
    return cb_cacaoConsts.DepthPrecisionOffsetMod;
}
FfxFloat32 NegRecEffectRadius()
{
    return cb_cacaoConsts.NegRecEffectRadius;
}
FfxFloat32 LoadCounterAvgDiv()
{
    return cb_cacaoConsts.LoadCounterAvgDiv;
}
FfxFloat32 AdaptiveSampleCountLimit()
{
    return cb_cacaoConsts.AdaptiveSampleCountLimit;
}
FfxFloat32 InvSharpness()
{
    return cb_cacaoConsts.InvSharpness;
}
FfxInt32 BlurNumPasses()
{
    return cb_cacaoConsts.BlurNumPasses;
}
FfxFloat32 BilateralSigmaSquared()
{
    return cb_cacaoConsts.BilateralSigmaSquared;
}
FfxFloat32 BilateralSimilarityDistanceSigma()
{
    return cb_cacaoConsts.BilateralSimilarityDistanceSigma;
}
FfxFloat32x4 PatternRotScaleMatrices(const in uint i, const in uint j)
{
    return cb_cacaoConsts.PatternRotScaleMatrices[i][j];
}
FfxFloat32 NormalsUnpackMul()
{
    return cb_cacaoConsts.NormalsUnpackMul;
}
FfxFloat32 NormalsUnpackAdd()
{
    return cb_cacaoConsts.NormalsUnpackAdd;
}
FfxFloat32 DetailAOStrength()
{
    return cb_cacaoConsts.DetailAOStrength;
}
FfxFloat32 Dummy0()
{
    return cb_cacaoConsts.Dummy0;
}
FfxFloat32x2 SSAOBufferDimensions()
{
    return cb_cacaoConsts.SSAOBufferDimensions;
}
FfxFloat32x2 SSAOBufferInverseDimensions()
{
    return cb_cacaoConsts.SSAOBufferInverseDimensions;
}
FfxFloat32x2 DepthBufferDimensions()
{
    return cb_cacaoConsts.DepthBufferDimensions;
}
FfxFloat32x2 DepthBufferInverseDimensions()
{
    return cb_cacaoConsts.DepthBufferInverseDimensions;
}
FfxInt32x2 DepthBufferOffset()
{
    return cb_cacaoConsts.DepthBufferOffset;
}
FfxFloat32x4 PerPassFullResUVOffset(const in uint i)
{
    return cb_cacaoConsts.PerPassFullResUVOffset[i];
}
FfxFloat32x2 OutputBufferDimensions()
{
    return cb_cacaoConsts.OutputBufferDimensions;
}
FfxFloat32x2 OutputBufferInverseDimensions()
{
    return cb_cacaoConsts.OutputBufferInverseDimensions;
}
FfxFloat32x2 ImportanceMapDimensions()
{
    return cb_cacaoConsts.ImportanceMapDimensions;
}
FfxFloat32x2 ImportanceMapInverseDimensions()
{
    return cb_cacaoConsts.ImportanceMapInverseDimensions;
}
FfxFloat32x2 DeinterleavedDepthBufferDimensions()
{
    return cb_cacaoConsts.DeinterleavedDepthBufferDimensions;
}
FfxFloat32x2 DeinterleavedDepthBufferInverseDimensions()
{
    return cb_cacaoConsts.DeinterleavedDepthBufferInverseDimensions;
}
FfxFloat32x2 DeinterleavedDepthBufferOffset()
{
    return cb_cacaoConsts.DeinterleavedDepthBufferOffset;
}
FfxFloat32x2 DeinterleavedDepthBufferNormalisedOffset()
{
    return cb_cacaoConsts.DeinterleavedDepthBufferNormalisedOffset;
}
float4x4 NormalsWorldToViewspaceMatrix()
{
    return cb_cacaoConsts.NormalsWorldToViewspaceMatrix;
}

#endif // CONSTANTS_CACAO_CS_HLSLI