cbuffer HDRCorrectionBuffer : register(b2)
{
	float cb_gamma;
	float cb_gammaInv;
	float cb_hdrExposure;
}

float3 DeleteGammaCorr(const in float3 srgb)
{
	return pow(srgb, float3(cb_gamma, cb_gamma, cb_gamma));
}

float3 AddGammaCorr(const in float3 srgb)
{
	return pow(srgb, float3(cb_gammaInv, cb_gammaInv, cb_gammaInv));
}