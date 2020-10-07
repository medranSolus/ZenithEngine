#include "BiasPB.hlsli"

#ifdef _TEX
SamplerState splr : register(s0);
Texture2D tex : register(t0);
#endif

float main(float3 lightToVertex : VECTOR
#ifdef _TEX
	, float2 tc : TEXCOORD
#endif
) : SV_TARGET
{
#ifdef _TEX
	clip(tex.Sample(splr, tc).a - 0.0039f);
#endif
	return length(lightToVertex) + cb_bias;
}