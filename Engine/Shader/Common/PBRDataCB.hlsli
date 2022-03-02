#include "CBuffer.hlsli"

static const uint SSAO_KERNEL_MAX_SIZE = 32;
static const uint BLUR_KERNEL_MAX_SIZE = 8;

struct PBRData
{
	float3 AmbientLight;
	float HDRExposure;

	uint2 FrameDimmensions;
	uint2 SsaoNoiseDimmensions;

	float SsaoBias;
	float SsaoSampleRadius;
	float SsaoPower;
	uint SsaoKernelSize;

	// Must not exceed coefficients size
	int BlurRadius;
	uint BlurWidth;
	uint BlurHeight;
	float BlurIntensity;

	float ShadowMapSize;
	float ShadowBias;
	float ShadowNormalOffset;
	float Gamma;

	float GammaInverse;

	float3 SsaoKernel[SSAO_KERNEL_MAX_SIZE];
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