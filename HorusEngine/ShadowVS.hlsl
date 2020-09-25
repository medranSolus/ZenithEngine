#include "TransformCBuffer.hlsli"

cbuffer CameraBuffer : register (b1)
{
	float3 cb_cameraPos;
};

struct VSOut
{
	float3 length : LENGTH;
	float4 pos : SV_POSITION;
};

VSOut main(float3 pos : POSITION)
{
	VSOut vso;
	// Vertex position relative to camera
	vso.pos = mul(float4(pos, 1.0f), cb_transformViewProjection);

	const float3 worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	vso.length = length(worldPos - cb_cameraPos);
	return vso;
}