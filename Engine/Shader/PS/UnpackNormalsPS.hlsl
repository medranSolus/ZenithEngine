#include "Buffers.hlsli"
#include "GBufferUtils.hlsli"
#include "Samplers.hlsli"

TEXTURE_EX(normalMap, Texture2D<CodedNormalGB>, 0, 0);

float4 main(float2 tc : TEXCOORD) : SV_TARGET0
{
	return float4(DecodeNormal(tx_normalMap.Sample(splr_PR, tc)), 0.0f);
}