#include "CBuffer.hlsli"

static const uint SSAO_KERNEL_MAX_SIZE = 32;

struct PBRData
{
	matrix ViewProjection;
	matrix ViewProjectionInverse;

	float NearClip;
	float FarClip;
	float Gamma;
	float GammaInverse;

	uint2 FrameDimmensions;
	float HDRExposure;
	struct
	{
		float Bias;

		uint2 NoiseDimmensions;
		float SampleRadius;
		float Power;

		float3 Kernel[SSAO_KERNEL_MAX_SIZE];

		uint KernelSize;
	} SSAO;
	struct
	{
		// Must not exceed coefficients size
		int Radius;
		uint Width;
		uint Height;

		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		float Coefficients[8];
		float Intensity;
	} Blur;
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

// Convert depth value from logarithmic depth space to linear view space
float GetLinearDepth(const in float depth)
{
	return cb_pbrData.NearClip * cb_pbrData.FarClip / (cb_pbrData.FarClip + depth * (cb_pbrData.NearClip - cb_pbrData.FarClip));
}