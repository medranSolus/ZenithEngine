#include "GammaCBuffer.hlsli"

SamplerState splr : register(s0);
TextureCube box : register(t0);

float4 main(float3 worldPos : POSITION) : SV_TARGET
{
	return float4(pow(box.Sample(splr, worldPos).rgb, float3(cb_deGamma, cb_deGamma, cb_deGamma)), 1.0f);
}