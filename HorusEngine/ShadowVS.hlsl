#include "TransformCBuffer.hlsli"

cbuffer CameraBuffer : register (b1)
{
	float3 cb_cameraPos;
};

float4 main(float3 pos : POSITION) : SV_POSITION
{
	// Vertex position relative to camera
	float4 position = mul(float4(pos, 1.0f), cb_transformViewProjection);
	// Depth is linear length from light, not just Z (premultiply by .w so no perspective divide)
	const float3 worldPos = mul(float4(pos, 1.0f), cb_transform).xyz;
	const float len = sqrt(length(worldPos - cb_cameraPos));
	position.z = len / (len + 1.0f) * position.w;
	return position;
}