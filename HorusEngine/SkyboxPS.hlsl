#include "HDRGammaPB.hlsli"

SamplerState splr : register(s0);
TextureCube box : register(t0);

float4 main(float3 worldPos : POSITION) : SV_TARGET
{
	return float4(DeleteGammaCorr(box.Sample(splr, worldPos).rgb), 1.0f);
}