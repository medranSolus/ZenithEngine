#include "TransformCBuffer.hlsli"

cbuffer CameraBuffer : register (b1)
{
	float3 cb_cameraPos;
};

struct VSOut
{
	float3 lightToVertex : VECTOR;
#ifdef _TEX
	float2 tc : TEXCOORD;
#endif
	float3 worldPos : POSITION;
};

VSOut main(float3 pos : POSITION
#ifdef _TEX
	, float3 normal : NORMAL, float2 tc : TEXCOORD
#endif
)
{
	VSOut vso;
	vso.worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.lightToVertex = vso.worldPos - cb_cameraPos;

#ifdef _TEX
	vso.tc = tc;
#endif
	return vso;
}