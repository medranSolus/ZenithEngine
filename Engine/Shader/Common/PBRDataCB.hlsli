#ifndef PBR_DATA_CB_HLSLI
#define PBR_DATA_CB_HLSLI
#include "Buffers.hlsli"
#include "../Include/XeGTAO.h"

#define ZE_BLUR_KERNEL_MAX_SIZE 8

struct PBRData
{
	uint2 DisplaySize;
	uint2 RenderSize;

	float3 AmbientLight;
	float HDRExposure;

	uint BlurWidth;
	uint BlurHeight;
	// Must not exceed coefficients size
	int BlurRadius;
	float BlurIntensity;
	
	float ShadowMapSize;
	float ShadowBias;
	float ShadowNormalOffset;
	float MipBias;
	
	float Gamma;
	float GammaInverse;
	
	// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
	float BlurCoefficients[ZE_BLUR_KERNEL_MAX_SIZE];
};
CBUFFER_GLOBAL(pbrData, PBRData, 13, 0);

float3 DeleteGammaCorr(const in float3 srgb)
{
	return pow(srgb, float3(cb_pbrData.Gamma, cb_pbrData.Gamma, cb_pbrData.Gamma));
}

float3 AddGammaCorr(const in float3 rgb)
{
	return pow(rgb, float3(cb_pbrData.GammaInverse, cb_pbrData.GammaInverse, cb_pbrData.GammaInverse));
}

#endif // PBR_DATA_CB_HLSLI