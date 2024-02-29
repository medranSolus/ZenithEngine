#define ZE_TRANSFORM_CB_RANGE 4
#include "TransformCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float4 worldTan : TANGENTPACK;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float4 tangent : TANGENTPACK)
{
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform);

	vso.tc = tc;
	vso.worldTan = float4(mul(tangent.xyz, (float3x3) cb_transform), tangent.w);

	return vso;
}