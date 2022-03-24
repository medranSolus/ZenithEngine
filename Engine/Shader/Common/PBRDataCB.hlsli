#ifndef PBR_DATA_CB_HLSLI
#define PBR_DATA_CB_HLSLI
#include "CBuffer.hlsli"
#include "../Include/XeGTAO.h"

static const uint BLUR_KERNEL_MAX_SIZE = 8;

struct PBRData
{
	// Should be multiple of 16 B (alignment restrictions)
	GTAOConstants SsaoData;

	float3 AmbientLight;
	float HDRExposure;

	uint BlurWidth;
	uint BlurHeight;
	// Must not exceed coefficients size
	int BlurRadius;
	float BlurIntensity;

	// LOW - 1, MEDIUM - 2, HIGH - 3, ULTRA - 9 (MAX is 9, otherwise change loop unroll value according to "CS/Utils/SSAO.hlsli")
	float SsaoSliceCount;
	// LOW - 2, MEDIUM - 2, HIGH - 3, ULTRA - 3 (MAX is 3, otherwise change loop unroll value according to "CS/Utils/SSAO.hlsli")
	float SsaoStepsPerSlice;
	float Gamma;
	float GammaInverse;

	float ShadowMapSize;
	float ShadowBias;
	float ShadowNormalOffset;

	// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
	float BlurCoefficients[BLUR_KERNEL_MAX_SIZE];
};
CBUFFER_GLOBAL(pbrData, PBRData, 13);

float3 DeleteGammaCorr(const in float3 srgb)
{
	return pow(srgb, float3(cb_pbrData.Gamma, cb_pbrData.Gamma, cb_pbrData.Gamma));
}

float3 AddGammaCorr(const in float3 rgb)
{
	return pow(rgb, float3(cb_pbrData.GammaInverse, cb_pbrData.GammaInverse, cb_pbrData.GammaInverse));
}

#endif // PBR_DATA_CB_HLSLI