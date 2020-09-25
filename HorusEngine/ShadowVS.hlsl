#include "TransformCBuffer.hlsli"

cbuffer CameraBuffer : register (b1)
{
	float3 cb_cameraPos;
};

struct VSOut
{
	float length : LENGTH;
#ifdef _TEX
	float2 tc : TEXCOORD;
#endif
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION
#ifdef _TEX
	, float3 normal : NORMAL, float2 tc : TEXCOORD
#endif
)
{
	VSOut vso;
	// Vertex position relative to camera
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

	const float3 worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.length = length(worldPos - cb_cameraPos);

#ifdef _TEX
	vso.tc = tc;
#endif
	return vso;
}