#include "TransformCBuffer.hlsli"

struct VSOut
{
	float4 color : COLOR;
	float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITION, float4 color : COLOR)
{
	VSOut vso;
	vso.color = color;
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);
	return vso;
}