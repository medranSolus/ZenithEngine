#include "Samplers.hlsli"
#include "Buffers.hlsli"

TEX2D(frame, 0, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 hdrColor = tx_frame.Sample(splr_PR, tc);
	// Reinhard tone mapping (favor for bright areas)
	const float3 mapped = hdrColor.rgb / (hdrColor.rgb + float3(1.0f, 1.0f, 1.0f));
	return float4(mapped, hdrColor.a);
}