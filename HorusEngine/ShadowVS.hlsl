#include "TransformCBuffer.hlsli"

float4 main(float3 pos : POSITION) : SV_POSITION
{
	// Vertex position relative to camera
	float4 position = mul(float4(pos, 1.0f), cb_transformViewProjection);
	// Depth is linear length from light, not just Z (premultiply by .w so no perspective divide)
	const float len = sqrt(length(mul(float4(pos, 1.0f), cb_transform).xyz));
	position.z = len / (len + 1.0f) * position.w;
	return position;
}