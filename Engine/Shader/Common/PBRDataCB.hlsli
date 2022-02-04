#include "CBuffer.hlsli"

struct PBRData
{
	matrix ViewProjection;
	matrix ViewProjectionInverse;
	float NearClip;
	float FarClip;
	float Gamma;
	float GammaInverse;
	float HDRExposure;

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