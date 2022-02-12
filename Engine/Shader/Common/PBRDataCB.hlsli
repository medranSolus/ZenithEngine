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

	float3 AmbientLight;
	float HDRExposure;

	uint2 FrameDimmensions;
	struct
	{
		uint2 NoiseDimmensions;

		float Bias;
		float SampleRadius;
		float Power;
		uint KernelSize;

		float3 Kernel[SSAO_KERNEL_MAX_SIZE];
	} SSAO;

	struct
	{
		// Must not exceed coefficients size
		int Radius;
		uint Width;
		uint Height;
		float Intensity;

		// Should be 6 * sigma - 1, current sigma for best effect 1.3 (but with reduced render target can be 2.6)
		float Coefficients[8];
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