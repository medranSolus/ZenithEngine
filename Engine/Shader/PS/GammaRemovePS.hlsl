#include "Buffers.hlsli"
#include "Samplers.hlsli"
#include "Utils.hlsli"

TEX2D(frame, 0, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	const float4 srgb = tx_frame.Sample(splr_PR, tc);
	return float4(DeleteGamma(srgb.rgb), srgb.a);
}