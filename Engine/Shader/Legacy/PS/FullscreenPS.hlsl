#include "Samplers.hlsli"

Texture2D tex : register(t0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	return tex.Sample(splr_PW, tc).rgba;
}