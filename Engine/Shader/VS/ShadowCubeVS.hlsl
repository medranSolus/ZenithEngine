#define TRANSFORM_RANGE 4
#include "TransformCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	float3 worldTan : TANGENT;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float3 tangent : TANGENT)
{
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform);

	vso.tc = tc;
	vso.worldTan = mul(tangent, (float3x3) cb_transform);

	return vso;
}