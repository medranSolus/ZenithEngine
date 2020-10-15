#include "HDRGammaPB.hlsli"

SamplerState splr : register(s0);
Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 hdrColor = tex.Sample(splr, tc).rgba;
	// Reinhard tone mapping (favor for bright areas)
	// TODO: Implement http://cs.brown.edu/courses/cs129/results/proj5/njooma/ as HDR image processing (requires bilateral filter http://people.csail.mit.edu/sparis/bf_course/)
	const float3 mapped = float3(1.0f, 1.0f, 1.0f) - exp(hdrColor.rgb * -cb_hdrExposure);
	return float4(AddGammaCorr(mapped), hdrColor.a);
}