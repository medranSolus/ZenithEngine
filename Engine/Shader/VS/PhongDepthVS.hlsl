#include "CB/Transform.hlsli"

float4 main(/*float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float3 bitangent : BITANGENT*/) : SV_POSITION
{
	float3 pos = 0.0f;

	return mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex].MVP);
}