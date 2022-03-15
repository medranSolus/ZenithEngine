#include "TransformCB.hlsli"

struct VSOut
{
	float3 worldPos : POSITION;
	float3 worldNormal : NORMAL;
	float2 tc : TEXCOORD;
	noperspective float4 worldTan : TANGENTPACK;
};

VSOut main(float3 pos : POSITION,
	float3 normal : NORMAL,
	float2 tc : TEXCOORD,
	float4 tangent : TANGENTPACK)
{
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform.Transforms[cb_transformIndex]).xyz;
	vso.worldNormal = mul(normal, (float3x3) cb_transform.Transforms[cb_transformIndex]);

	vso.tc = tc;
	vso.worldTan = float4(mul(tangent.xyz, (float3x3) cb_transform.Transforms[cb_transformIndex]), tangent.w);

	return vso;
}