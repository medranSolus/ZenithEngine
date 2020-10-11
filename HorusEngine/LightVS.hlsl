#include "TransformVB.hlsli"

struct VSOut
{
	float2 tc : TEXCOORD;
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);
	vso.tc = float2((vso.pos.x / vso.pos.w + 1.0f) / 2.0f, (vso.pos.y / vso.pos.w - 1.0f) / -2.0f);
	return vso;
}