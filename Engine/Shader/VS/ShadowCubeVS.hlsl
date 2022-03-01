#include "TransformCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float3 worldBitan : BITANGENT;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float3 bitangent : BITANGENT)
{
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex]).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform.Transforms[cb_transformIndex]);

	vso.tc = tc;
	vso.worldBitan = mul(bitangent, (float3x3) cb_transform.Transforms[cb_transformIndex]);

	return vso;
}