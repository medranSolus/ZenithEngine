#include "Samplers.hlsli"
#include "PBRDataCB.hlsli"

TEX2D(frame, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 hdrColor = tx_frame.Sample(splr_PR, tc);
	// Reinhard tone mapping (favor for bright areas)
	// TODO: Implement http://cs.brown.edu/courses/cs129/results/proj5/njooma/
	//		 as HDR image processing (requires bilateral filter http://people.csail.mit.edu/sparis/bf_course/)
	const float3 mapped = float3(1.0f, 1.0f, 1.0f) - exp(hdrColor.rgb * -cb_pbrData.HDRExposure);
	return float4(AddGammaCorr(mapped), hdrColor.a);
}