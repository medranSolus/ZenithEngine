#include "TransformCBuffer.hlsli"

float4 main(float3 pos : POSITION) : SV_POSITION
{
	// Vertex position relative to camera
	float4 position = mul(float4(pos, 1.0f), cb_transformViewProjection);
	// Depth is linear length from light, not just Z (premultiply by .w so no perspective divide)
	// Far clip plane at 1000.0f
	position.z = length(mul(float4(pos, 1.0f), cb_transformView).xyz) * position.w / 1000.0f;
	return position;
}