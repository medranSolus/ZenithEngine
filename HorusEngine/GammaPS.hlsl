#include "GammaPB.hlsli"

SamplerState splr : register(s0);
Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	float4 pixel = tex.Sample(splr, tc).rgba;
	return float4(pow(pixel.rgb, float3(cb_gamma, cb_gamma, cb_gamma)), pixel.a);
}